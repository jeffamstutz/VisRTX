// Copyright 2024-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ToneMapPass.h"
// tsd_algorithms
#include "tsd/algorithms/cpu/toneMap.hpp"
#if TSD_ALGORITHMS_HAS_CUDA
#include "tsd/algorithms/cuda/toneMap.hpp"
#endif
// std
#include <cmath>

namespace tsd::rendering {

// ToneMapPass definitions /////////////////////////////////////////////////////

ToneMapPass::ToneMapPass() = default;

ToneMapPass::~ToneMapPass() = default;

void ToneMapPass::setOperator(ToneMapOperator op)
{
  m_operator = op;
}

void ToneMapPass::setAutoExposureEnabled(bool enabled)
{
  m_autoExposureEnabled = enabled;
}

void ToneMapPass::setExposure(float exposure)
{
  m_exposure = exposure;
}

void ToneMapPass::setHDREnabled(bool enabled)
{
  m_hdrEnabled = enabled;
}

void ToneMapPass::render(ImageBuffers &b, int stageId)
{
  if (stageId == 0 || !m_hdrEnabled)
    return;

  const auto size = getDimensions();
  const uint32_t totalPixels = size.x * size.y;
  if (totalPixels == 0 || !b.hdrColor)
    return;

  const float exposure =
      (m_autoExposureEnabled ? b.exposure : 0.f) + m_exposure;
  const float exposureScale = std::exp2(exposure);

#if TSD_ALGORITHMS_HAS_CUDA
  if (b.stream) {
    tsd::algorithms::cuda::toneMap(
        b.stream, b.hdrColor, totalPixels, exposureScale, m_operator);
    return;
  }
#endif
  tsd::algorithms::cpu::toneMap(
      b.hdrColor, totalPixels, exposureScale, m_operator);
}

} // namespace tsd::rendering
