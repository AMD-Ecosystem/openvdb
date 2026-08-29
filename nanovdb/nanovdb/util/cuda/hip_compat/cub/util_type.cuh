// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// HIP-path forwarding header for <cub/util_type.cuh>: GridStats.cuh uses
// cub::Uninitialized as constructor-free __shared__ scratch. hipCUB provides
// hipcub::Uninitialized under <hipcub/util_type.hpp>; the cuda_to_hip.h shim
// aliases cub:: -> hipcub:: so the source spelling stays unchanged.
#ifndef NANOVDB_HIP_COMPAT_CUB_UTIL_TYPE_CUH
#define NANOVDB_HIP_COMPAT_CUB_UTIL_TYPE_CUH
#include <hipcub/hipcub.hpp>
#endif
