## Copyright 2026 NVIDIA Corporation
## SPDX-License-Identifier: Apache-2.0

project(tsdDataTreeEditor)
project_add_executable(DataTreeWindow.cpp tsdDataTreeEditor.cpp)
project_link_libraries(tsd_ui_imgui)
