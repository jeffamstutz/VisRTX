// Copyright 2024-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "VisualizeAOVPass.h"
// tsd_algorithms
#include "tsd/algorithms/cpu/visualizeAOV.hpp"
#ifdef TSD_ALGORITHMS_HAS_CUDA
#include "tsd/algorithms/cuda/visualizeAOV.hpp"
#endif

namespace tsd::rendering {

// VisualizeAOVPass definitions ///////////////////////////////////////////////

VisualizeAOVPass::VisualizeAOVPass() = default;

VisualizeAOVPass::~VisualizeAOVPass() = default;

void VisualizeAOVPass::setAOVType(AOVType type)
{
  m_aovType = type;
  setEnabled(type != AOVType::NONE);
}

void VisualizeAOVPass::setDepthRange(float minDepth, float maxDepth)
{
  m_minDepth = minDepth;
  m_maxDepth = maxDepth;
}

void VisualizeAOVPass::setEdgeInvert(bool invert)
{
  m_edgeInvert = invert;
}

void VisualizeAOVPass::render(ImageBuffers &b, int stageId)
{
  if (stageId == 0 || m_aovType == AOVType::NONE)
    return;

  const auto size = getDimensions();

#ifdef TSD_ALGORITHMS_HAS_CUDA
  if (b.stream) {
    namespace alg = tsd::algorithms::cuda;
    switch (m_aovType) {
    case AOVType::DEPTH:
      alg::visualizeDepth(
          b.stream, b.depth, b.color, m_minDepth, m_maxDepth, size.x, size.y);
      break;
    case AOVType::ALBEDO:
      alg::visualizeAlbedo(b.stream, b.albedo, b.color, size.x, size.y);
      break;
    case AOVType::NORMAL:
      alg::visualizeNormal(b.stream, b.normal, b.color, size.x, size.y);
      break;
    case AOVType::EDGES:
      alg::visualizeEdges(
          b.stream, b.objectId, b.color, m_edgeInvert, size.x, size.y);
      break;
    case AOVType::OBJECT_ID:
      alg::visualizeId(b.stream, b.objectId, b.color, size.x, size.y);
      break;
    case AOVType::PRIMITIVE_ID:
      alg::visualizeId(b.stream, b.primitiveId, b.color, size.x, size.y);
      break;
    case AOVType::INSTANCE_ID:
      alg::visualizeId(b.stream, b.instanceId, b.color, size.x, size.y);
      break;
    default:
      break;
    }
    return;
  }
#endif

  namespace alg = tsd::algorithms::cpu;
  switch (m_aovType) {
  case AOVType::DEPTH:
    alg::visualizeDepth(
        b.depth, b.color, m_minDepth, m_maxDepth, size.x, size.y);
    break;
  case AOVType::ALBEDO:
    alg::visualizeAlbedo(b.albedo, b.color, size.x, size.y);
    break;
  case AOVType::NORMAL:
    alg::visualizeNormal(b.normal, b.color, size.x, size.y);
    break;
  case AOVType::EDGES:
    alg::visualizeEdges(b.objectId, b.color, m_edgeInvert, size.x, size.y);
    break;
  case AOVType::OBJECT_ID:
    alg::visualizeId(b.objectId, b.color, size.x, size.y);
    break;
  case AOVType::PRIMITIVE_ID:
    alg::visualizeId(b.primitiveId, b.color, size.x, size.y);
    break;
  case AOVType::INSTANCE_ID:
    alg::visualizeId(b.instanceId, b.color, size.x, size.y);
    break;
  default:
    break;
  }
}

} // namespace tsd::rendering
