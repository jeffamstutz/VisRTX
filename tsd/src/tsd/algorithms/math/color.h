// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "device_macros.h"
#include "tsd/core/TSDMath.hpp"
// std
#include <algorithm>
#include <cmath>

namespace tsd::algorithms::math {

TSD_HOST_DEVICE_FCN inline float luminance(float r, float g, float b)
{
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

TSD_DEVICE_FCN_INLINE tsd::math::float3 linearToGamma(
    tsd::math::float3 c, float invGamma)
{
  c.x = std::pow(std::clamp(c.x, 0.f, 1.f), invGamma);
  c.y = std::pow(std::clamp(c.y, 0.f, 1.f), invGamma);
  c.z = std::pow(std::clamp(c.z, 0.f, 1.f), invGamma);
  return c;
}

// Deterministic pseudo-random color from an ID (same as visrtx gpu_utils.h)
TSD_DEVICE_FCN inline tsd::math::float3 makeRandomColor(uint32_t i)
{
  const uint32_t mx = 13 * 17 * 43;
  const uint32_t my = 11 * 29;
  const uint32_t mz = 7 * 23 * 63;
  const uint32_t g = (i * (3 * 5 * 127) + 12312314);

  if (i == ~0u)
    return tsd::math::float3(0.0f);

  return tsd::math::float3((g % mx) * (1.f / (mx - 1)),
      (g % my) * (1.f / (my - 1)),
      (g % mz) * (1.f / (mz - 1)));
}

} // namespace tsd::algorithms::math
