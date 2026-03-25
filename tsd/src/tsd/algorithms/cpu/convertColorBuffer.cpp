// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "tsd/algorithms/cpu/convertColorBuffer.hpp"
#include "detail/parallel_for.h"
// std
#include <algorithm>

namespace tsd::algorithms::cpu {

void convertFloatToUint8(const float *src, uint8_t *dst, size_t count)
{
  detail::parallel_for(0u, uint32_t(count), [=](uint32_t i) {
    dst[i] = uint8_t(std::clamp(src[i], 0.f, 1.f) * 255);
  });
}

} // namespace tsd::algorithms::cpu
