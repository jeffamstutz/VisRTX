// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#ifdef ENABLE_TBB
#include <tbb/parallel_for.h>
#endif

#include <cstdint>

namespace tsd::algorithms::cpu::detail {

template <typename FCN>
inline void parallel_for(uint32_t start, uint32_t end, FCN &&fcn)
{
#ifdef ENABLE_TBB
  tbb::parallel_for(start, end, fcn);
#else
  for (auto i = start; i < end; i++)
    fcn(i);
#endif
}

} // namespace tsd::algorithms::cpu::detail
