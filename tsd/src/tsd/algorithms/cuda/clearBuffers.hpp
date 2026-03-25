// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cuda_runtime_api.h>
#include <cstdint>

namespace tsd::algorithms::cuda {

void fill(cudaStream_t stream, uint32_t *buf, uint32_t count, uint32_t value);
void fill(uint32_t *buf, uint32_t count, uint32_t value);

void fill(cudaStream_t stream, float *buf, uint32_t count, float value);
void fill(float *buf, uint32_t count, float value);

} // namespace tsd::algorithms::cuda
