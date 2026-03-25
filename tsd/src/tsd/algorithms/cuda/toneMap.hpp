// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cuda_runtime_api.h>
#include <cstdint>
#include "tsd/algorithms/cpu/toneMap.hpp" // ToneMapOperator enum

namespace tsd::algorithms::cuda {

void toneMap(cudaStream_t stream,
    float *hdrColor,
    uint32_t numPixels,
    float exposureScale,
    ToneMapOperator op);

void toneMap(float *hdrColor,
    uint32_t numPixels,
    float exposureScale,
    ToneMapOperator op);

} // namespace tsd::algorithms::cuda
