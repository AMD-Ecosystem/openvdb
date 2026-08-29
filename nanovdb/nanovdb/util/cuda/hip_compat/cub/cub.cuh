// Copyright (C) 2026 Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// HIP-path forwarding header: NanoVDB device sources spell their device
// collectives as <cub/cub.cuh> + cub::Device*. On ROCm there is no cub/ tree;
// hipCUB ships the same public Device* API under <hipcub/hipcub.hpp> in the
// hipcub:: namespace. This header (only on the include path when NANOVDB_USE_HIP
// is ON) resolves the CUDA-spelled include to hipCUB and the cuda_to_hip.h shim
// aliases the cub:: namespace to hipcub::, so the NanoVDB sources stay unchanged.
//
// KB: cub-hipcub, hipcub-device-collective-returns-error-code.
#ifndef NANOVDB_HIP_COMPAT_CUB_CUB_CUH
#define NANOVDB_HIP_COMPAT_CUB_CUB_CUH
#include <hipcub/hipcub.hpp>
#endif
