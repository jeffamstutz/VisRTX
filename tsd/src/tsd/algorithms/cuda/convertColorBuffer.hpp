// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cuda_runtime_api.h>
#include <cstddef>
#include <cstdint>

namespace tsd::algorithms::cuda {

void convertFloatToUint8(
    cudaStream_t stream, const float *src, uint8_t *dst, size_t count);
void convertFloatToUint8(const float *src, uint8_t *dst, size_t count);

} // namespace tsd::algorithms::cuda
