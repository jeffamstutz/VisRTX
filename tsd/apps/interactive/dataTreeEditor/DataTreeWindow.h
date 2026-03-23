// Copyright 2024-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

// tsd_ui_imgui
#include <tsd/ui/imgui/windows/Window.h>
// tsd_core
#include <tsd/core/DataTree.hpp>
// std
#include <string>
#include <vector>

namespace tsd::datatree_editor {

class DataTreeWindow : public tsd::ui::imgui::Window
{
 public:
  DataTreeWindow(tsd::ui::imgui::Application *app,
      tsd::core::DataTree *tree,
      bool *dirty,
      const std::string *currentFile);
  ~DataTreeWindow() override = default;

  void buildUI() override;

 private:
  // parent is nullptr only for immediate root children (their parent is root)
  void renderNode(tsd::core::DataNode &node, tsd::core::DataNode &parent);
  void renderValueEditor(tsd::core::DataNode &node);
  void applyPendingOps();

  tsd::core::DataTree *m_tree{nullptr};
  bool *m_dirty{nullptr};
  const std::string *m_currentFile{nullptr};

  // Deferred structural edits — target is always the *parent* node.
  // For AddChild: append a child named 'name' to target.
  // For Delete:   remove the child named 'name' from target.
  struct PendingOp
  {
    enum Type { AddChild, Delete } type;
    tsd::core::DataNode *target{nullptr};
    std::string name;
  };
  std::vector<PendingOp> m_pendingOps;

  // State for "Add Child" modal
  tsd::core::DataNode *m_addChildTarget{nullptr};
  std::string m_addChildName;
};

} // namespace tsd::datatree_editor
