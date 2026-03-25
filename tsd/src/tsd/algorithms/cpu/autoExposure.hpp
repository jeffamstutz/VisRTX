// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace tsd::algorithms::cpu {

// Compute sum of log2(luminance) over strided samples from an interleaved
// RGBA float buffer. Returns the raw sum — caller divides by numSamples
// and applies exp2 to get average luminance.
float sumLogLuminance(
    const float *hdrColor, uint32_t numSamples, uint32_t stride);

} // namespace tsd::algorithms::cpu
