// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "tsd/algorithms/cpu/outline.hpp"
#include "detail/parallel_for.h"
// tsd_core
#include "tsd/core/TSDMath.hpp"
// helium
#include <helium/helium_math.h>
// std
#include <algorithm>

namespace tsd::algorithms::cpu {

namespace {

inline uint32_t shadePixel(uint32_t c_in)
{
  auto c_in_f = helium::cvt_color_to_float4(c_in);
  auto c_h = tsd::math::float4(1.f, 0.5f, 0.f, 1.f);
  auto c_out = tsd::math::lerp(c_in_f, c_h, 0.8f);
  return helium::cvt_color_to_uint32(c_out);
}

} // namespace

void outline(const uint32_t *objectId,
    uint32_t *color,
    uint32_t outlineId,
    uint32_t width,
    uint32_t height)
{
  detail::parallel_for(0u, width * height, [=](uint32_t i) {
    uint32_t y = i / width;
    uint32_t x = i % width;

    int cnt = 0;
    for (unsigned fy = std::max(0u, y - 1); fy <= std::min(height - 1, y + 1);
        fy++) {
      for (unsigned fx = std::max(0u, x - 1); fx <= std::min(width - 1, x + 1);
          fx++) {
        size_t fi = fx + size_t(width) * fy;
        if (objectId[fi] == outlineId)
          cnt++;
      }
    }

    if (cnt > 1 && cnt < 8)
      color[i] = shadePixel(color[i]);
  });
}

} // namespace tsd::algorithms::cpu
