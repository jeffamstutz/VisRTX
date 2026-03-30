// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace tsd::algorithms::cpu {

void depthCompositeFrame(uint32_t *outColor,
    float *outDepth,
    uint32_t *outObjectId,
    const uint32_t *inColor,
    const float *inDepth,
    const uint32_t *inObjectId,
    uint32_t pixelCount,
    bool firstPass);

} // namespace tsd::algorithms::cpu
