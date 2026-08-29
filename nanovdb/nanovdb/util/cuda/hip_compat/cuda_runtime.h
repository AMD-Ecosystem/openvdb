// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// HIP-path forwarding header for <cuda_runtime.h>: NanoVDB device sources include the CUDA
// runtime headers directly. On ROCm these resolve (via the NANOVDB_USE_HIP include
// path) to this shim, which pulls in the HIP runtime and the cuda_to_hip.h symbol
// remap so the CUDA-spelled sources compile unchanged. KB:
// cuda-to-hip-single-shim-header-keep-cu-extension.
#ifndef NANOVDB_HIP_COMPAT_CUDA_RUNTIME_H_INCLUDED
#define NANOVDB_HIP_COMPAT_CUDA_RUNTIME_H_INCLUDED
#include <hip/hip_runtime.h>
#include <hip/hip_runtime_api.h>
#include <nanovdb/util/cuda/cuda_to_hip.h>
#endif
