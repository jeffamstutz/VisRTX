// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstdint>

namespace tsd::algorithms::cpu {

void fill(uint32_t *buf, uint32_t count, uint32_t value);
void fill(float *buf, uint32_t count, float value);

} // namespace tsd::algorithms::cpu
