// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Modal.h"

namespace tsd::ui::imgui {

struct CuttingPlaneDialog : public Modal
{
  CuttingPlaneDialog(Application *app);
  ~CuttingPlaneDialog() override = default;

  void buildUI() override;

 private:
  bool  m_enabled{false};
  float m_normal[3]{0.f, 0.f, 1.f};
  float m_d{0.f};
};

} // namespace tsd::ui::imgui
