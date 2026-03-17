// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// tsd_rendering
#include "tsd/rendering/pipeline/passes/ImagePass.h"

namespace tsd::rendering {

/*
 * ImagePass that copies a caller-owned byte vector into the pipeline's
 * internal color buffer; useful for injecting externally produced frames.
 *
 * Example:
 *   std::vector<uint8_t> externalPixels = receive();
 *   auto *pass = pipeline.emplace_back<CopyToColorBufferPass>();
 *   pass->setExternalBuffer(externalPixels);
 */
struct CopyToColorBufferPass : public ImagePass
{
  CopyToColorBufferPass();
  ~CopyToColorBufferPass() override;

  void setExternalBuffer(std::vector<uint8_t> &buffer);

 private:
  void render(ImageBuffers &b, int stageId) override;

  std::vector<uint8_t> *m_externalBuffer{nullptr};
};

} // namespace tsd::rendering
