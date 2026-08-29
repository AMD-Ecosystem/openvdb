// Copyright Contributors to the OpenVDB Project
// Modifications Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

/*!
    \file nanovdb/util/cuda/cuda_to_hip.h

    \brief Single CUDA -> HIP compatibility shim for the NanoVDB GPU tooling layer.

    Compatibility-header approach: the NanoVDB device tooling headers keep their CUDA
    spelling (cudaXxx, cub::, thrust::) and the .cu/.cuh extension; when compiled by
    hipcc (which defines __HIPCC__ / __HIP_PLATFORM_AMD__) this header remaps every
    CUDA runtime / library token NanoVDB uses onto its HIP peer so the same sources
    compile unchanged. On nvcc this header is a no-op include.

    Included from nanovdb/util/cuda/Util.h before the first use, so every translation
    unit that pulls in a NanoVDB cuda header sees the remap. Any missing CUDA symbol
    is a hard compile error -- grow the map here iteratively from build failures.

    KB: cuda-to-hip-single-shim-header-keep-cu-extension, cub-hipcub, thrust-rocthrust,
        hip-mempool-async-alloc, cudamallocmanaged-hipmallocmanaged, hip-pinned-host-malloc,
        hipcub-device-collective-returns-error-code.
*/

#ifndef NANOVDB_UTIL_CUDA_CUDA_TO_HIP_H_HAS_BEEN_INCLUDED
#define NANOVDB_UTIL_CUDA_CUDA_TO_HIP_H_HAS_BEEN_INCLUDED

#if defined(__HIP_PLATFORM_AMD__) || defined(__HIPCC__)

// rocThrust must select the HIP device backend BEFORE any thrust header is pulled in.
#ifndef THRUST_DEVICE_SYSTEM
#define THRUST_DEVICE_SYSTEM THRUST_DEVICE_SYSTEM_HIP
#endif

#include <hip/hip_runtime.h>
#include <hip/hip_runtime_api.h>

// ---------------------------------------------------------------------------
// Runtime: types + error codes
// ---------------------------------------------------------------------------
#define cudaError_t                         hipError_t
#define cudaSuccess                         hipSuccess
#define cudaErrorNotSupported               hipErrorNotSupported
#define cudaErrorNotReady                   hipErrorNotReady
#define cudaStream_t                        hipStream_t
#define cudaEvent_t                         hipEvent_t
#define cudaDeviceProp                      hipDeviceProp_t
#define cudaPointerAttributes               hipPointerAttribute_t
#define cudaMemcpyKind                      hipMemcpyKind
#define cudaMemoryAdvise                    hipMemoryAdvise
#define cudaMemPool_t                       hipMemPool_t
#define cudaMemPoolProps                    hipMemPoolProps
#define cudaMemPoolAttr                     hipMemPoolAttr
#define cudaMemLocation                     hipMemLocation
#define cudaMemLocationType                 hipMemLocationType
#define cudaFuncAttribute                   hipFuncAttribute

// Sentinels / enum values
#define cudaInvalidDeviceId                 hipInvalidDeviceId
#define cudaCpuDeviceId                     hipCpuDeviceId
#define cudaMemcpyHostToDevice              hipMemcpyHostToDevice
#define cudaMemcpyDeviceToHost              hipMemcpyDeviceToHost
#define cudaMemcpyDeviceToDevice           hipMemcpyDeviceToDevice
#define cudaMemcpyHostToHost                hipMemcpyHostToHost
#define cudaMemcpyDefault                   hipMemcpyDefault
#define cudaEventDisableTiming             hipEventDisableTiming
#define cudaEventDefault                    hipEventDefault
#define cudaEventBlockingSync              hipEventBlockingSync
#define cudaStreamNonBlocking             hipStreamNonBlocking
#define cudaStreamDefault                  hipStreamDefault
#define cudaStreamPerThread                hipStreamPerThread
#define cudaMemAttachGlobal                hipMemAttachGlobal
#define cudaMemAttachHost                  hipMemAttachHost
#define cudaMemAttachSingle               hipMemAttachSingle
#define cudaMemLocationTypeInvalid          hipMemLocationTypeInvalid
#define cudaMemLocationTypeHost             hipMemLocationTypeHost
#define cudaMemLocationTypeDevice           hipMemLocationTypeDevice
#define cudaDevAttrMemoryPoolsSupported     hipDeviceAttributeMemoryPoolsSupported
#define cudaDevAttrClockRate                hipDeviceAttributeClockRate
#define cudaFuncAttributeMaxDynamicSharedMemorySize hipFuncAttributeMaxDynamicSharedMemorySize

// Advise enums used by NanoVDB memory-resource code
#define cudaMemAdviseSetPreferredLocation   hipMemAdviseSetPreferredLocation
#define cudaMemAdviseSetAccessedBy          hipMemAdviseSetAccessedBy
#define cudaMemAdviseSetReadMostly          hipMemAdviseSetReadMostly

// ---------------------------------------------------------------------------
// Runtime: query / device management
// ---------------------------------------------------------------------------
#define cudaGetErrorString                  hipGetErrorString
#define cudaGetLastError                    hipGetLastError
#define cudaGetDevice                       hipGetDevice
#define cudaSetDevice                       hipSetDevice
#define cudaGetDeviceCount                  hipGetDeviceCount
#define cudaGetDeviceProperties             hipGetDeviceProperties
#define cudaDeviceGetAttribute              hipDeviceGetAttribute
#define cudaPointerGetAttributes            hipPointerGetAttributes
#define cudaDeviceSynchronize               hipDeviceSynchronize
#define cudaFuncSetAttribute                hipFuncSetAttribute

// ---------------------------------------------------------------------------
// Runtime: memory
// ---------------------------------------------------------------------------
#define cudaMalloc                          hipMalloc
#define cudaFree                            hipFree
#define cudaMallocAsync                     hipMallocAsync
#define cudaFreeAsync                       hipFreeAsync
#define cudaMallocManaged                   hipMallocManaged
#define cudaMallocHost                      hipHostMalloc
#define cudaFreeHost                        hipHostFree
#define cudaMemcpy                          hipMemcpy
#define cudaMemcpyAsync                     hipMemcpyAsync
#define cudaMemset                          hipMemset
#define cudaMemsetAsync                     hipMemsetAsync
#define cudaMemAdvise                       hipMemAdvise
#define cudaMemPrefetchAsync                hipMemPrefetchAsync
#define cudaMemGetInfo                      hipMemGetInfo

// Stream-ordered memory pools (hip mempool 1:1)
#define cudaMemPoolCreate                   hipMemPoolCreate
#define cudaMemPoolDestroy                  hipMemPoolDestroy
#define cudaMemPoolSetAttribute             hipMemPoolSetAttribute
#define cudaMemPoolGetAttribute             hipMemPoolGetAttribute
#define cudaMallocFromPoolAsync             hipMallocFromPoolAsync
#define cudaDeviceGetDefaultMemPool         hipDeviceGetDefaultMemPool
#define cudaDeviceSetMemPool                hipDeviceSetMemPool

// ---------------------------------------------------------------------------
// Runtime: streams + events
// ---------------------------------------------------------------------------
#define cudaStreamCreate                    hipStreamCreate
#define cudaStreamCreateWithFlags           hipStreamCreateWithFlags
#define cudaStreamDestroy                   hipStreamDestroy
#define cudaStreamSynchronize               hipStreamSynchronize
#define cudaStreamQuery                     hipStreamQuery
#define cudaStreamWaitEvent                 hipStreamWaitEvent
#define cudaEventCreate                     hipEventCreate
#define cudaEventCreateWithFlags            hipEventCreateWithFlags
#define cudaEventDestroy                    hipEventDestroy
#define cudaEventRecord                     hipEventRecord
#define cudaEventSynchronize                hipEventSynchronize
#define cudaEventElapsedTime                hipEventElapsedTime

// ---------------------------------------------------------------------------
// Runtime: stream capture + graphs (unit tests)
// ---------------------------------------------------------------------------
#define cudaGraph_t                         hipGraph_t
#define cudaGraphExec_t                     hipGraphExec_t
#define cudaGraphDestroy                    hipGraphDestroy
#define cudaGraphExecDestroy                hipGraphExecDestroy
#define cudaGraphLaunch                     hipGraphLaunch
#define cudaStreamBeginCapture              hipStreamBeginCapture
#define cudaStreamEndCapture                hipStreamEndCapture
#define cudaStreamCaptureModeGlobal         hipStreamCaptureModeGlobal
#define cudaMemoryTypeHost                  hipMemoryTypeHost

// CUDART_VERSION drives NanoVDB's mempool/prefetch version gates. HIP always ships
// the stream-ordered pool + the (device-id) prefetch signature, so pin CUDART_VERSION
// to a value that selects the async-mempool path AND the pre-CUDA-13 prefetch/advise
// signature (int device id, not cudaMemLocation) that maps 1:1 onto the HIP API.
#ifndef CUDART_VERSION
#define CUDART_VERSION 12000
#endif

// ---------------------------------------------------------------------------
// Libraries: cub -> hipCUB, thrust -> rocThrust
//   NanoVDB spells device collectives as <cub/cub.cuh> + cub::Device*. On the HIP
//   path CMake adds util/cuda/hip_compat to the include path so <cub/cub.cuh> and
//   <cub/util_type.cuh> resolve to forwarding headers that pull in hipCUB, and the
//   namespace ALIAS below maps cub:: -> hipcub:: so the source spelling is unchanged.
//   A namespace alias (not a #define) is used so it never rewrites the include token.
//
//   hipCUB device collectives return hipError_t just like CUB returns cudaError_t,
//   so NanoVDB's CALL_CUBS(...) macro -- which already wraps each call in
//   cudaCheck(...) (now hipCheck) -- stays return-code-safe with no rewrite.
//   KB: hipcub-device-collective-returns-error-code, cub-hipcub, thrust-rocthrust.
// ---------------------------------------------------------------------------
// cudaGraphInstantiate has diverging arities: the modern CUDA form is
// (pGraphExec, graph, flags) while hipGraphInstantiate is
// (pGraphExec, graph, pErrorNode, pLogBuffer, bufferSize). A token #define cannot
// bridge that, so map the 3-arg CUDA spelling with an inline forwarder that drops the
// (unused-in-test) flags and passes the HIP error/log outputs as null. Kept as a real
// inline function (not a macro) so the CUDA-spelled call site is unchanged.
static inline hipError_t cudaGraphInstantiate(hipGraphExec_t* pGraphExec, hipGraph_t graph,
                                              unsigned long long flags = 0)
{
    (void)flags;
    return hipGraphInstantiate(pGraphExec, graph, nullptr, nullptr, 0);
}

#include <hipcub/hipcub.hpp>
namespace cub = hipcub;

// ---------------------------------------------------------------------------
// Deferred surfaces (single-GPU-first scope)
//   Two NanoVDB surfaces are NOT ported in this first single-GPU pass because
//   they ride heavier, separable dependencies with no runtime-shim entry above:
//     - nanovdb/cuda/DeviceStreamMap.h : the CUDA Driver-API virtual-memory
//       management path (CUmemAllocationProp / CU_MEM_* / CUresult, obtained via
//       cudaGetDriverEntryPoint). This is a driver-API VMM surface, not a runtime
//       API, so it needs a dedicated hipMem* / hipDrv driver-VMM pass.
//     - nanovdb/cuda/DeviceMesh.h : NCCL multi-GPU comms (ncclComm_t /
//       ncclCommInitAll). Maps to RCCL, but multi-GPU is a separable deliverable.
//   NANOVDB_HIP_DEFER_DRIVER_API is defined here (unless NANOVDB_USE_NCCL turns
//   the multi-GPU surface on) so headers and unit tests can guard the deferred
//   includes/test-cases OUT of the HIP build cleanly, with a documented reason,
//   rather than half-porting un-shimmed CU*/nccl* symbols into the primary target.
//   Removing this scope means porting the driver-VMM path + wiring RCCL and
//   dropping the guard. See VALIDATION_NOTES.md "Deferred surfaces".
// ---------------------------------------------------------------------------
#if !defined(NANOVDB_USE_NCCL) && !defined(NANOVDB_HIP_DEFER_DRIVER_API)
#define NANOVDB_HIP_DEFER_DRIVER_API 1
#endif

#endif // __HIP_PLATFORM_AMD__ || __HIPCC__

#endif // NANOVDB_UTIL_CUDA_CUDA_TO_HIP_H_HAS_BEEN_INCLUDED
