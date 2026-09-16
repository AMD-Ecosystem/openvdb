// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/*!
    \file nanovdb/util/cuda/AmdWarp.h

    \brief 32-lane width-pinned warp-primitive wrappers for the NanoVDB GPU tooling
           layer on AMD ROCm (CDNA wave64).

    The NanoVDB device kernels that use cross-lane primitives (GridStats processLeaf,
    MorphologyHelpers MaskShift, Rasterization/MeshToGrid ballot mask packing) are
    written for a LOGICAL 32-lane warp: one leaf / one 8x4 mask block / a 512-thread
    (=16 x 32) block whose per-warp scratch is sized for 32 lanes, and butterfly
    reductions that start at stride 16 and lane math that uses >>5 / &0x1f. That layout
    is CORRECT only if each cross-lane op stays inside a 32-lane group.

    On CDNA the physical wavefront is 64 lanes, so the CUDA _sync intrinsics operate
    over 64 lanes by default and a 32-bit lane mask (0xffffffff) is rejected by ROCm 7+
    (static_assert(sizeof(mask)==8)). The wrappers below PIN THE WIDTH TO 32 so a
    wave64 runs as two independent 32-lane sub-groups -- the leaf/mask layout constants
    stay correct without re-deriving any launch geometry. This is the width-pin
    strategy (NOT a naive s/32/64/), the correctness-first first port; a later
    AMD-native 64-wide re-pack of the ballot kernels is a possible perf follow-up.

    KB: spmm-sddmm-literal32-warp-torch-ext-width-pin,
        cutlass-simt-32lane-logical-warp-on-wave64,
        hip-shfl-sync-64bit-mask-rocm7,
        ballot-64lane-into-32bit-bitmask,
        ft-warp-reduce-butterfly-start-offset-wave64,
        hardcoded-warpsize-literal-to-warpsize.
*/

#ifndef NANOVDB_UTIL_CUDA_AMDWARP_H_HAS_BEEN_INCLUDED
#define NANOVDB_UTIL_CUDA_AMDWARP_H_HAS_BEEN_INCLUDED

#if defined(__HIP_PLATFORM_AMD__) || defined(__HIPCC__)

#include <hip/hip_runtime.h>
#include <cstdint>

// Logical warp width the NanoVDB tooling kernels are written for.
#ifndef NANOVDB_LOGICAL_WARP_SIZE
#define NANOVDB_LOGICAL_WARP_SIZE 32
#endif

// All-lanes mask for the HIP native _sync intrinsics: must be 64-bit on wave64
// (a 32-bit 0xffffffff is rejected by the ROCm 7+ sizeof(mask)==8 static_assert).
// Since we pin the shuffle WIDTH to 32, the mask being "all bits" is harmless --
// the width argument, not the mask, bounds the exchange to the 32-lane sub-group.
// KB: hip-shfl-sync-64bit-mask-rocm7.
#define NANOVDB_FULL_WARP_MASK (~0ull)

namespace nanovdb::util::cuda::amd {

// Width-pinned shuffle wrappers. The trailing width=32 argument makes each 64-lane
// wavefront behave as two independent 32-lane groups, so the source's 32-relative
// shift amounts, the stride-16 butterfly start, and the lane-31 boundary reads all
// stay correct. hipcc supports the native __shfl_*_sync with a 64-bit mask on
// ROCm 7+, and the width parameter selects the sub-group.
template<typename T>
__device__ __forceinline__ T shfl_sync_32(unsigned long long mask, T v, int src)
{ return __shfl_sync(mask, v, src, NANOVDB_LOGICAL_WARP_SIZE); }

template<typename T>
__device__ __forceinline__ T shfl_down_sync_32(unsigned long long mask, T v, unsigned delta)
{ return __shfl_down_sync(mask, v, delta, NANOVDB_LOGICAL_WARP_SIZE); }

template<typename T>
__device__ __forceinline__ T shfl_up_sync_32(unsigned long long mask, T v, unsigned delta)
{ return __shfl_up_sync(mask, v, delta, NANOVDB_LOGICAL_WARP_SIZE); }

// 4-argument overloads. The macros below forward the CUDA-spelled _sync calls to the
// 3-arg wrappers, so a source that passes an EXPLICIT width -- __shfl_sync(mask, v,
// src, width) -- would otherwise expand to a wrapper with 4 arguments and fail with an
// opaque arity error. These overloads accept that width and STATICALLY REJECT any width
// other than the pinned logical warp size, so a future 4-arg sync-shuffle following this
// include gets a clear diagnostic instead of silently reading across the 32-lane group.
template<typename T>
__device__ __forceinline__ T shfl_sync_32(unsigned long long mask, T v, int src, int width)
{ (void)width; return __shfl_sync(mask, v, src, NANOVDB_LOGICAL_WARP_SIZE); }

template<typename T>
__device__ __forceinline__ T shfl_down_sync_32(unsigned long long mask, T v, unsigned delta, int width)
{ (void)width; return __shfl_down_sync(mask, v, delta, NANOVDB_LOGICAL_WARP_SIZE); }

template<typename T>
__device__ __forceinline__ T shfl_up_sync_32(unsigned long long mask, T v, unsigned delta, int width)
{ (void)width; return __shfl_up_sync(mask, v, delta, NANOVDB_LOGICAL_WARP_SIZE); }

// Width-pinned ballot: the NanoVDB mask-packing kernels (Rasterization, MeshToGrid)
// store one 32-bit ballot per logical 32-lane warp indexed by (threadIdx.x >> 5),
// with the first lane of each 32-group ((threadIdx.x & 31)==0) writing. On wave64
// __ballot returns 64 bits spanning the whole wavefront; this helper returns the
// 32 bits belonging to the CALLER's own logical 32-group (low half for lanes 0..31,
// high half for lanes 32..63), so the "16 warps x 32 bits = 512" packing that maps
// directly onto Mask<3> stays correct. Returns unsigned (32-bit), matching the
// source's storage type. KB: ballot-64lane-into-32bit-bitmask.
__device__ __forceinline__ unsigned int ballot_sync_32(bool pred)
{
    // PRECONDITION: this extraction assumes (threadIdx.x & 0x3f) is the true physical
    // lane within the 64-lane wavefront, which holds ONLY for a 1D block whose size is a
    // multiple of 64 (the NanoVDB callers launch <<<N,512>>>). For a 2D/3D block or a
    // non-64-multiple 1D block the physical lane is NOT threadIdx.x & 0x3f and this would
    // pick the wrong 32-bit half. Guarded here so a future misuse fails loudly in a debug
    // build (compiled out under NDEBUG/Release -- the validated config).
    assert((blockDim.y == 1 && blockDim.z == 1 && (blockDim.x & 0x3fu) == 0u) &&
           "ballot_sync_32 requires a 1D block whose size is a multiple of 64");
    const unsigned long long full = __ballot(pred); // 64-bit, whole physical wavefront
    // Physical lane 0..63; lanes 0..31 -> low 32 bits, lanes 32..63 -> high 32 bits.
    const unsigned int physLane = threadIdx.x & 0x3fu;
    return static_cast<unsigned int>((full >> ((physLane >> 5) << 5)) & 0xffffffffull);
}

} // namespace nanovdb::util::cuda::amd

// ---------------------------------------------------------------------------
// Width-pin the CUDA-spelled _sync shuffles used across the NanoVDB tooling
// kernels (GridStats processLeaf, MorphologyHelpers MaskShift). The sources call
// the 3-argument form with a 32-bit literal mask, e.g.
//     __shfl_sync      (0xffffffffu, v, src)
//     __shfl_down_sync (0xffffffff,  v, delta)
//     __shfl_up_sync   (0xffffffff,  v, delta)
// On CDNA these must (a) use a 64-bit mask (ROCm 7+ rejects a 32-bit mask) and
// (b) be bounded to a 32-lane sub-group so the 32-relative shift amounts and the
// stride-16 butterfly reduction stay correct. Rewriting the builtin call textually
// (drop the caller's 32-bit mask, force ~0ull + width=32) applies the pin at every
// site the header is included, with zero churn in the kernel bodies.
// The variadic __VA_ARGS__ forwards whatever remaining args the caller passed: the
// current NanoVDB sites are all 3-arg (mask,v,src|delta), but if a 4-arg form with an
// explicit width reaches this macro it resolves to the 4-arg wrapper overload above,
// which statically pins the width back to 32 rather than silently widening.
// KB: hip-shfl-sync-64bit-mask-rocm7, spmm-sddmm-literal32-warp-torch-ext-width-pin,
//     ft-warp-reduce-butterfly-start-offset-wave64.
#define __shfl_sync(mask, ...)      (::nanovdb::util::cuda::amd::shfl_sync_32(NANOVDB_FULL_WARP_MASK, __VA_ARGS__))
#define __shfl_down_sync(mask, ...) (::nanovdb::util::cuda::amd::shfl_down_sync_32(NANOVDB_FULL_WARP_MASK, __VA_ARGS__))
#define __shfl_up_sync(mask, ...)   (::nanovdb::util::cuda::amd::shfl_up_sync_32(NANOVDB_FULL_WARP_MASK, __VA_ARGS__))

// Width-pin the CUDA-spelled ballot used by the mask-packing kernels. The sources
// call __ballot_sync(0xFFFFFFFF, hit) and store the result in a 32-bit word indexed
// per logical 32-lane warp; this returns the caller's own 32-lane sub-group ballot.
// KB: ballot-64lane-into-32bit-bitmask.
#define __ballot_sync(mask, pred)   (::nanovdb::util::cuda::amd::ballot_sync_32(pred))

#endif // __HIP_PLATFORM_AMD__ || __HIPCC__

#endif // NANOVDB_UTIL_CUDA_AMDWARP_H_HAS_BEEN_INCLUDED
