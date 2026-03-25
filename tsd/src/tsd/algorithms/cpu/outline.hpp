// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace tsd::algorithms::cpu {

// Highlight edges of a selected object by blending an orange tint
// on pixels where the 3x3 neighborhood straddles the object boundary.
void outline(const uint32_t *objectId,
    uint32_t *color,
    uint32_t outlineId,
    uint32_t width,
    uint32_t height);

} // namespace tsd::algorithms::cpu
