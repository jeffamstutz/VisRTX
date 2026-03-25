// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>
#include <cstdint>

namespace tsd::algorithms::cpu {

void convertFloatToUint8(const float *src, uint8_t *dst, size_t count);

} // namespace tsd::algorithms::cpu
