// Copyright 2025-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "Timeline.h"
// tsd_core
#include "tsd/core/ObjectPool.hpp"
#include "tsd/core/scene/Animation.hpp"
#include "tsd/core/scene/objects/Camera.hpp"
// tsd_ui_imgui
#include "tsd/ui/imgui/Application.h"
// std
#include <algorithm>
#include <cmath>
#include <cstring>

namespace tsd::ui::imgui {

static void captureCurrentCameraKeyframe(
    tsd::core::Animation *anim, float t)
{
  const auto *obj = anim->keyframeTargetObject();
  if (!obj || obj->type() != ANARI_CAMERA)
    return;
  const auto *cam = static_cast<const tsd::core::Camera *>(obj);

  if (auto v = cam->parameterValueAs<math::float3>("position"))
    anim->addValueKeyframe("position", ANARI_FLOAT32_VEC3, t, &*v);
  if (auto v = cam->parameterValueAs<math::float3>("direction"))
    anim->addValueKeyframe("direction", ANARI_FLOAT32_VEC3, t, &*v);
  if (auto v = cam->parameterValueAs<math::float3>("up"))
    anim->addValueKeyframe("up", ANARI_FLOAT32_VEC3, t, &*v);
  if (cam->subtype() == tsd::core::tokens::camera::perspective) {
    if (auto v = cam->parameterValueAs<float>("fovy"))
      anim->addValueKeyframe("fovy", ANARI_FLOAT32, t, &*v);
  }
}

Timeline::Timeline(Application *app, const char *name) : Window(app, name) {}

void Timeline::buildUI()
{
  auto *core = appCore();
  auto &scene = core->tsd.scene;

  // Space toggles play
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
      && ImGui::IsKeyPressed(ImGuiKey_Space)) {
    m_playing = !m_playing;
  }

  if (m_playing) {
    scene.incrementAnimationFrame();
    if (!m_loop && scene.getAnimationFrame() == 0)
      m_playing = false;
  }

  // K shortcut: set keyframe on selected tracks
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)
      && ImGui::IsKeyPressed(ImGuiKey_K)) {
    float t = scene.getAnimationTime();
    for (size_t i = 0; i < scene.numberOfAnimations(); i++) {
      if (!m_selectedTracks.empty() && m_selectedTracks.count(i) == 0)
        continue;
      auto *anim = scene.animation(i);
      if (!anim->hasKeyframes() && anim->timeStepCount() > 0)
        continue; // skip baked timestep animations
      auto nodeRef = anim->keyframeTargetNode();
      if (nodeRef) {
        math::mat4 mat = (*nodeRef)->getTransform();
        anim->addTransformKeyframe(t, mat);
      } else {
        captureCurrentCameraKeyframe(anim, t);
      }
    }
  }

  buildUI_transport();
  ImGui::Separator();
  buildUI_canvas();
}

void Timeline::buildUI_transport()
{
  auto *core = appCore();
  auto &scene = core->tsd.scene;

  // Play/Pause
  if (m_playing) {
    if (ImGui::Button("||"))
      m_playing = false;
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Pause (Space)");
  } else {
    if (ImGui::Button(" > "))
      m_playing = true;
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Play (Space)");
  }
  ImGui::SameLine();

  // Stop
  if (ImGui::Button(" [] ")) {
    m_playing = false;
    scene.setAnimationFrame(0);
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Stop");
  ImGui::SameLine();

  // Loop toggle
  ImGui::Checkbox("Loop", &m_loop);
  ImGui::SameLine();

  // Frame counter
  int frame = scene.getAnimationFrame();
  int totalFrames = scene.getAnimationTotalFrames();
  ImGui::SetNextItemWidth(60.f);
  if (ImGui::InputInt("##frame", &frame, 1, 10))
    scene.setAnimationFrame(frame);
  ImGui::SameLine();
  ImGui::Text("/ %d", totalFrames - 1);
  ImGui::SameLine();

  // Total frames
  ImGui::SetNextItemWidth(70.f);
  if (ImGui::InputInt("Frames", &totalFrames, 1, 10))
    scene.setAnimationTotalFrames(totalFrames);
  ImGui::SameLine();

  // FPS
  float fps = scene.getAnimationFPS();
  ImGui::SetNextItemWidth(70.f);
  if (ImGui::DragFloat("FPS", &fps, 0.5f, 1.f, 240.f, "%.1f"))
    scene.setAnimationFPS(fps);
}

void Timeline::buildUI_canvas()
{
  auto *core = appCore();
  auto &scene = core->tsd.scene;

  const float nameColWidth = 150.f;
  const float rowHeight = 20.f;
  const float rulerHeight = 24.f;

  int totalFrames = scene.getAnimationTotalFrames();
  int currentFrame = scene.getAnimationFrame();
  float canvasWidth = ImGui::GetContentRegionAvail().x - nameColWidth;

  // Zoom with scroll wheel when hovering canvas
  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
    float wheel = ImGui::GetIO().MouseWheel;
    if (wheel != 0.f && ImGui::GetIO().KeyCtrl) {
      m_pixelsPerFrame = std::clamp(m_pixelsPerFrame * (1.f + wheel * 0.1f),
          1.f,
          50.f);
    }
  }

  float totalCanvasWidth = m_pixelsPerFrame * (totalFrames - 1);

  // Left column: track names
  ImGui::BeginChild("##track_names",
      ImVec2(nameColWidth, ImGui::GetContentRegionAvail().y),
      false,
      ImGuiWindowFlags_NoScrollbar);
  {
    ImGui::Dummy(ImVec2(nameColWidth, rulerHeight)); // spacer for ruler row

    for (size_t i = 0; i < scene.numberOfAnimations(); i++) {
      auto *anim = scene.animation(i);
      ImGui::PushID(static_cast<int>(i));

      bool selected = m_selectedTracks.count(i) > 0;
      if (selected)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));

      // Truncate name
      char label[64];
      const auto &animName = anim->name();
      if (animName.size() > 20) {
        std::snprintf(label, sizeof(label), "%.17s...", animName.c_str());
      } else {
        std::snprintf(label, sizeof(label), "%s", animName.c_str());
      }

      if (ImGui::Button(label, ImVec2(nameColWidth - 20.f, rowHeight))) {
        if (ImGui::GetIO().KeyCtrl) {
          if (selected)
            m_selectedTracks.erase(i);
          else
            m_selectedTracks.insert(i);
        } else {
          m_selectedTracks.clear();
          m_selectedTracks.insert(i);
        }
      }

      if (selected)
        ImGui::PopStyleColor();

      // Remove button
      ImGui::SameLine();
      if (ImGui::SmallButton("x")) {
        scene.removeAnimation(anim);
        m_selectedTracks.erase(i);
        ImGui::PopID();
        break; // iterator invalidated
      }

      ImGui::PopID();
    }

    // Add Track
    if (ImGui::Button("+ Add Track", ImVec2(nameColWidth, 0))) {
      ImGui::OpenPopup("##add_track_popup");
    }

    if (ImGui::BeginPopup("##add_track_popup")) {
      ImGui::Text("Select a transform node to animate:");
      ImGui::Separator();

      for (size_t li = 0; li < scene.numberOfLayers(); li++) {
        auto *layer = scene.layer(li);
        if (!layer)
          continue;

        int nodeIdx = 0;
        layer->traverse(layer->root(), [&](tsd::core::LayerNode &node, int level) {
          // Show transform nodes and named nodes (skip pure object/leaf nodes)
          if (!node->isTransform() && node->name().empty())
            return true;

          // Build a label: use name if set, else a fallback
          char label[128];
          if (!node->name().empty()) {
            std::snprintf(label, sizeof(label), "%*s%s", level * 2, "", node->name().c_str());
          } else {
            std::snprintf(label, sizeof(label), "%*s[xform %d]", level * 2, "", nodeIdx);
          }
          nodeIdx++;

          ImGui::PushID(static_cast<int>(li * 100000 + node.index()));
          if (ImGui::MenuItem(label)) {
            auto nodeRef = layer->at(node.index());
            const char *animName = !node->name().empty() ? node->name().c_str() : label;
            scene.addKeyframeAnimation(animName, nodeRef);
            ImGui::CloseCurrentPopup();
          }
          ImGui::PopID();
          return true;
        });
      }

      ImGui::Separator();
      ImGui::Text("Cameras:");

      const auto &cameraDB = scene.objectDB().camera;
      tsd::core::foreach_item_const(cameraDB, [&](const auto *cam) {
        if (!cam)
          return;
        char label[128];
        const auto &camName = cam->name();
        if (!camName.empty())
          std::snprintf(label, sizeof(label), "%s", camName.c_str());
        else
          std::snprintf(label, sizeof(label), "Camera [%zu]", cam->index());
        ImGui::PushID(static_cast<int>(cam->index() + 900000));
        if (ImGui::MenuItem(label)) {
          scene.addKeyframeAnimationForCamera(label, cam->self());
          ImGui::CloseCurrentPopup();
        }
        ImGui::PopID();
      });

      ImGui::EndPopup();
    }
  }
  ImGui::EndChild();

  ImGui::SameLine();

  // Right canvas: ruler + keyframe tracks
  ImGui::BeginChild("##timeline_canvas",
      ImVec2(canvasWidth, ImGui::GetContentRegionAvail().y),
      false,
      ImGuiWindowFlags_HorizontalScrollbar);
  {
    // Reserve space for all content
    size_t numAnims = scene.numberOfAnimations();
    float contentHeight = rulerHeight + numAnims * rowHeight + rowHeight + 4.f;
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(std::max(canvasWidth, totalCanvasWidth + 20.f),
        contentHeight));

    ImDrawList *draw = ImGui::GetWindowDrawList();

    // Ruler background
    draw->AddRectFilled(
        canvasPos,
        ImVec2(canvasPos.x + std::max(canvasWidth, totalCanvasWidth + 20.f),
            canvasPos.y + rulerHeight),
        IM_COL32(50, 50, 50, 255));

    // Ruler ticks
    const int tickInterval =
        m_pixelsPerFrame < 4.f ? 20 : (m_pixelsPerFrame < 10.f ? 10 : 5);
    for (int f = 0; f < totalFrames; f += tickInterval) {
      float x = canvasPos.x + f * m_pixelsPerFrame;
      draw->AddLine(
          ImVec2(x, canvasPos.y + rulerHeight - 8.f),
          ImVec2(x, canvasPos.y + rulerHeight),
          IM_COL32(200, 200, 200, 255));
      char buf[16];
      std::snprintf(buf, sizeof(buf), "%d", f);
      draw->AddText(
          ImVec2(x + 2.f, canvasPos.y + 2.f), IM_COL32(200, 200, 200, 255), buf);
    }

    // Scrubber (draggable vertical line)
    float scrubX = canvasPos.x + currentFrame * m_pixelsPerFrame;
    draw->AddLine(
        ImVec2(scrubX, canvasPos.y),
        ImVec2(scrubX, canvasPos.y + contentHeight),
        IM_COL32(255, 80, 80, 255),
        2.f);

    // Check for scrubber drag
    ImVec2 scrubMin(scrubX - 6.f, canvasPos.y);
    ImVec2 scrubMax(scrubX + 6.f, canvasPos.y + rulerHeight);
    ImGui::SetCursorScreenPos(scrubMin);
    ImGui::InvisibleButton(
        "##scrubber", ImVec2(scrubMax.x - scrubMin.x, scrubMax.y - scrubMin.y));
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
      float dx = ImGui::GetIO().MouseDelta.x;
      int newFrame =
          std::clamp(currentFrame + static_cast<int>(dx / m_pixelsPerFrame),
              0,
              totalFrames - 1);
      scene.setAnimationFrame(newFrame);
    }

    // Click on ruler to seek
    ImGui::SetCursorScreenPos(canvasPos);
    ImGui::InvisibleButton(
        "##ruler_seek",
        ImVec2(std::max(canvasWidth, totalCanvasWidth + 20.f), rulerHeight));
    if (ImGui::IsItemClicked(0)) {
      float mx = ImGui::GetMousePos().x - canvasPos.x;
      int newFrame = std::clamp(
          static_cast<int>(mx / m_pixelsPerFrame), 0, totalFrames - 1);
      scene.setAnimationFrame(newFrame);
    }

    // Animation rows
    for (size_t i = 0; i < scene.numberOfAnimations(); i++) {
      auto *anim = scene.animation(i);
      float rowY = canvasPos.y + rulerHeight + i * rowHeight;

      // Row background (alternating)
      uint32_t rowBg = (i % 2 == 0) ? IM_COL32(40, 40, 40, 200)
                                     : IM_COL32(35, 35, 35, 200);
      draw->AddRectFilled(
          ImVec2(canvasPos.x, rowY),
          ImVec2(canvasPos.x + std::max(canvasWidth, totalCanvasWidth + 20.f),
              rowY + rowHeight),
          rowBg);

      // Draw transform keyframe diamonds
      const auto &tkfs = anim->transformKeyframes();
      for (size_t ki = 0; ki < tkfs.size(); ki++) {
        float kx = canvasPos.x + tkfs[ki].time * (totalFrames - 1) * m_pixelsPerFrame;
        float ky = rowY + rowHeight * 0.5f;
        float r = 5.f;

        bool isSelected = m_selectedTracks.count(i) > 0;
        uint32_t fillCol = isSelected ? IM_COL32(255, 200, 50, 255)
                                      : IM_COL32(200, 160, 30, 255);

        // Diamond shape
        draw->AddQuadFilled(
            ImVec2(kx, ky - r),
            ImVec2(kx + r, ky),
            ImVec2(kx, ky + r),
            ImVec2(kx - r, ky),
            fillCol);
        draw->AddQuad(ImVec2(kx, ky - r),
            ImVec2(kx + r, ky),
            ImVec2(kx, ky + r),
            ImVec2(kx - r, ky),
            IM_COL32(255, 255, 255, 180));

        // Interaction
        ImGui::SetCursorScreenPos(ImVec2(kx - r, ky - r));
        ImGui::PushID(static_cast<int>(i * 10000 + ki));
        ImGui::InvisibleButton("##kf", ImVec2(r * 2.f, r * 2.f));

        if (ImGui::IsItemHovered()) {
          int kframe = static_cast<int>(
              std::round(tkfs[ki].time * (totalFrames - 1)));
          ImGui::SetTooltip("Frame %d", kframe);
        }

        if (ImGui::IsItemClicked(0))
          scene.setAnimationFrame(
              static_cast<int>(std::round(tkfs[ki].time * (totalFrames - 1))));

        // Right-click context
        if (ImGui::BeginPopupContextItem("##kf_ctx")) {
          if (ImGui::MenuItem("Delete keyframe"))
            anim->removeTransformKeyframe(ki);
          if (ImGui::MenuItem("Move to current frame"))
            anim->addTransformKeyframe(scene.getAnimationTime(),
                tkfs[ki].matrix); // addTransformKeyframe handles replace
          ImGui::EndPopup();
        }

        // Drag to move
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
          float dx = ImGui::GetIO().MouseDelta.x;
          float newTime = tkfs[ki].time + dx / ((totalFrames - 1) * m_pixelsPerFrame);
          newTime = std::clamp(newTime, 0.f, 1.f);
          math::mat4 mat = tkfs[ki].matrix;
          anim->removeTransformKeyframe(ki);
          anim->addTransformKeyframe(newTime, mat);
        }

        ImGui::PopID();
      }

      // Draw value channel keyframe diamonds (different color)
      const auto &channels = anim->keyframeChannels();
      for (size_t ci = 0; ci < channels.size(); ci++) {
        const auto &chan = channels[ci];
        float subRowY = rowY + ci * (rowHeight / std::max<size_t>(1, channels.size()));
        for (size_t ki = 0; ki < chan.keyframes.size(); ki++) {
          float kx = canvasPos.x
              + chan.keyframes[ki].time * (totalFrames - 1) * m_pixelsPerFrame;
          float ky = rowY + rowHeight * 0.5f;
          float r = 4.f;
          draw->AddQuadFilled(
              ImVec2(kx, ky - r),
              ImVec2(kx + r, ky),
              ImVec2(kx, ky + r),
              ImVec2(kx - r, ky),
              IM_COL32(80, 200, 255, 255));
          draw->AddQuad(ImVec2(kx, ky - r),
              ImVec2(kx + r, ky),
              ImVec2(kx, ky + r),
              ImVec2(kx - r, ky),
              IM_COL32(200, 240, 255, 180));

          ImGui::SetCursorScreenPos(ImVec2(kx - r, ky - r));
          ImGui::PushID(static_cast<int>(i * 10000 + 5000 + ci * 100 + ki));
          ImGui::InvisibleButton("##vkf", ImVec2(r * 2.f, r * 2.f));
          if (ImGui::IsItemHovered()) {
            int kframe = static_cast<int>(
                std::round(chan.keyframes[ki].time * (totalFrames - 1)));
            ImGui::SetTooltip(
                "%s @ frame %d", chan.parameterName.c_str(), kframe);
          }
          if (ImGui::BeginPopupContextItem("##vkf_ctx")) {
            if (ImGui::MenuItem("Delete keyframe"))
              anim->removeValueKeyframe(chan.parameterName, ki);
            ImGui::EndPopup();
          }
          ImGui::PopID();
        }
      }
    }

    // Set Keyframe button
    float btnY = canvasPos.y + rulerHeight + scene.numberOfAnimations() * rowHeight + 4.f;
    ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, btnY));
    if (ImGui::Button("Set Keyframe (K)")) {
      float t = scene.getAnimationTime();
      for (size_t i = 0; i < scene.numberOfAnimations(); i++) {
        if (!m_selectedTracks.empty() && m_selectedTracks.count(i) == 0)
          continue;
        auto *anim = scene.animation(i);
        auto nodeRef = anim->keyframeTargetNode();
        if (nodeRef) {
          math::mat4 mat = (*nodeRef)->getTransform();
          anim->addTransformKeyframe(t, mat);
        } else {
          captureCurrentCameraKeyframe(anim, t);
        }
      }
    }
  }
  ImGui::EndChild();
}

} // namespace tsd::ui::imgui
