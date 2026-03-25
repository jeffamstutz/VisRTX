// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace tsd::algorithms {

enum class ToneMapOperator
{
  NONE,
  REINHARD,
  ACES,
  HABLE,
  KHRONOS_PBR_NEUTRAL,
  AGX,
};

namespace cpu {

// Apply tone mapping in-place to an interleaved RGBA float buffer.
// hdrColor: pointer to numPixels * 4 floats (RGBARGBA...).
// exposureScale: pre-computed multiplier (e.g., std::exp2(exposure)).
void toneMap(float *hdrColor,
    uint32_t numPixels,
    float exposureScale,
    ToneMapOperator op);

} // namespace cpu
} // namespace tsd::algorithms
