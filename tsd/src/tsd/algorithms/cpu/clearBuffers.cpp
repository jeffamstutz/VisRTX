// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "tsd/algorithms/cpu/clearBuffers.hpp"
// std
#include <algorithm>

namespace tsd::algorithms::cpu {

void fill(uint32_t *buf, uint32_t count, uint32_t value)
{
  std::fill(buf, buf + count, value);
}

void fill(float *buf, uint32_t count, float value)
{
  std::fill(buf, buf + count, value);
}

} // namespace tsd::algorithms::cpu
