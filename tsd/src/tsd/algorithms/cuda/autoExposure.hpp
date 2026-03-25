// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cuda_runtime_api.h>
#include <cstdint>

namespace tsd::algorithms::cuda {

float sumLogLuminance(cudaStream_t stream,
    const float *hdrColor,
    uint32_t numSamples,
    uint32_t stride);

float sumLogLuminance(
    const float *hdrColor, uint32_t numSamples, uint32_t stride);

} // namespace tsd::algorithms::cuda
