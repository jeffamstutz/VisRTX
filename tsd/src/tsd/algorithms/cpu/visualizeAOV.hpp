// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>
#include "tsd/core/TSDMath.hpp"

namespace tsd::algorithms::cpu {

void visualizeId(
    const uint32_t *id, uint32_t *color, uint32_t width, uint32_t height);
void visualizeDepth(const float *depth,
    uint32_t *color,
    float minDepth,
    float maxDepth,
    uint32_t width,
    uint32_t height);
void visualizeAlbedo(const tsd::math::float3 *albedo,
    uint32_t *color,
    uint32_t width,
    uint32_t height);
void visualizeNormal(const tsd::math::float3 *normal,
    uint32_t *color,
    uint32_t width,
    uint32_t height);
void visualizeEdges(const uint32_t *objectId,
    uint32_t *color,
    bool invert,
    uint32_t width,
    uint32_t height);

} // namespace tsd::algorithms::cpu
