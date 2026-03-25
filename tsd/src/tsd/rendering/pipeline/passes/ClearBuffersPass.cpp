// Copyright 2024-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ClearBuffersPass.h"
// tsd_algorithms
#include "tsd/algorithms/cpu/clearBuffers.hpp"
#if TSD_ALGORITHMS_HAS_CUDA
#include "tsd/algorithms/cuda/clearBuffers.hpp"
#endif
// std
#include <limits>

namespace tsd::rendering {

ClearBuffersPass::ClearBuffersPass() = default;
ClearBuffersPass::~ClearBuffersPass() = default;

void ClearBuffersPass::setClearColor(const tsd::math::float4 &color)
{
  m_clearColor = color;
}

void ClearBuffersPass::render(ImageBuffers &b, int /*stageId*/)
{
  const auto size = getDimensions();
  const uint32_t totalPixels = uint32_t(size.x) * uint32_t(size.y);
  const auto c = helium::cvt_color_to_uint32(m_clearColor);
  const float inf = std::numeric_limits<float>::infinity();

#if TSD_ALGORITHMS_HAS_CUDA
  if (b.stream) {
    using tsd::algorithms::cuda::fill;
    fill(b.stream, b.color, totalPixels, c);
    fill(b.stream, b.hdrColor, totalPixels * 4, 0.f);
    fill(b.stream, b.depth, totalPixels, inf);
    fill(b.stream, b.objectId, totalPixels, ~0u);
    fill(b.stream, b.primitiveId, totalPixels, ~0u);
    fill(b.stream, b.instanceId, totalPixels, ~0u);
    return;
  }
#endif
  using tsd::algorithms::cpu::fill;
  fill(b.color, totalPixels, c);
  fill(b.hdrColor, totalPixels * 4, 0.f);
  fill(b.depth, totalPixels, inf);
  fill(b.objectId, totalPixels, ~0u);
  fill(b.primitiveId, totalPixels, ~0u);
  fill(b.instanceId, totalPixels, ~0u);
}

} // namespace tsd::rendering
