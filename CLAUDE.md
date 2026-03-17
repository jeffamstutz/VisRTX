# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**VisRTX** is an experimental NVIDIA implementation of the [Khronos ANARI](https://www.khronos.org/anari) (Analytic Rendering Interface) standard, focused on scientific visualization. It ships two ANARI device implementations and an optional testing/demo framework (TSD).

## Build

### Requirements
- CMake 3.17+, C++17 compiler, NVIDIA Driver 530+
- CUDA 12+ (for RTX device), ANARI-SDK 0.15.0+

### Basic Build
```bash
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install /path/to/visrtx
cmake --build . --parallel
cmake --install .
```

### Key CMake Options
| Option | Default | Description |
|--------|---------|-------------|
| `VISRTX_BUILD_RTX_DEVICE` | ON | OptiX/CUDA ray-tracing device |
| `VISRTX_BUILD_GL_DEVICE` | ON | OpenGL 4.3/GLES 3.2 device |
| `VISRTX_BUILD_TSD` | OFF | TSD testing applications |
| `VISRTX_ENABLE_MDL_SUPPORT` | — | NVIDIA MDL material support |
| `VISRTX_ENABLE_NEURAL` | — | Neural Graphics Primitives |
| `VISRTX_ENABLE_NVTX` | — | NVTX profiling |

TSD has its own set of options (when `VISRTX_BUILD_TSD=ON`):
```
TSD_BUILD_APPS, TSD_BUILD_INTERACTIVE_APPS, TSD_BUILD_UI_LIBRARY,
TSD_USE_CUDA, TSD_USE_ASSIMP, TSD_USE_HDF5, TSD_USE_MPI,
TSD_USE_NETWORKING, TSD_USE_LUA, TSD_USE_VTK, TSD_USE_SILO, TSD_USE_USD
```

OptiX version can be pinned via `OPTIX_FETCH_VERSION` (supports 7.7, 8.0, 8.1, 9.0).

## Tests

TSD contains unit tests; run them with:
```bash
ctest -C Release --output-on-failure
# or a single test
ctest -C Release -R <test_name> --output-on-failure
```

Test sources are in `tsd/tests/`.

## Code Style

Google-style C++ formatting via `.clang-format`. C++17 throughout. Format with:
```bash
clang-format -i <file>
```

## Architecture

### ANARI Devices (`devices/`)

Both devices implement the ANARI C API and are installed as shared libraries loadable by any ANARI application.

- **`devices/rtx/`** — OptiX-based GPU ray tracer (`libanari_library_visrtx`). Implements 30+ Khronos and NVIDIA ANARI extensions including CUDA array/framebuffer interop, NanoVDB volumes, and MDL materials. The heavy lifting is in OptiX launch parameters, hit programs, and CUDA kernels.

- **`devices/gl/`** — OpenGL rasterizer (`libanari_library_visgl`). Simpler implementation; supports shadow mapping and ambient occlusion.

### TSD: Testing Scene Description (`tsd/`)

TSD is explicitly **experimental** — no API stability guarantees. It is educational and for testing ANARI devices, not a production pipeline.

**Core libraries** (always built):
- `tsd_core` — Foundational types: `Any`, `Token`, `ObjectPool`, `DataTree`, logging
- `tsd_scene` — ANARI-mirrored scene graph with layers (instancing forest), keyframe animation, and an `UpdateDelegate` notification interface
- `tsd_io` — 30+ file format importers/exporters and procedural generators
- `tsd_rendering` — `RenderIndex` (TSD↔ANARI sync), `ImagePipeline` (composable render passes), camera tools
- `tsd_app` — `ANARIDeviceManager`, CLI parsing, application context bundling
- `anari_tsd` — ANARI device that mirrors state back into TSD scenes

**Optional libraries**:
- `tsd_ui_imgui` — ImGui-based UI framework used by interactive apps
- `tsd_mpi` — MPI distributed state synchronization
- `tsd_network` — Boost.Asio client/server scene streaming
- `tsd_lua` — Lua 5.4 scripting with full TSD scene bindings (auto-fetches Lua + Sol3)

**Applications** (`tsd/apps/`):
- `tsdViewer` — Primary interactive viewer (ImGui, multi-device, layer editing)
- `tsdRender` / `tsdOffline` — CLI/headless renderers
- `tsdLua` — Lua REPL/interpreter for scene scripting
- `mpiViewer`, `tsdRemoteViewer`/`tsdServer` — Distributed/networked variants

### Key Design Patterns

- **ANARI mirroring**: TSD maintains its own scene graph that mirrors ANARI object state, enabling editing, serialization, and networking without being tied to device internals.
- **UpdateDelegate**: `BaseUpdateDelegate` is the notification interface between scene mutations and consumers (renderer, network, UI). Subclass this to react to scene changes.
- **RenderIndex**: Responsible for translating TSD scene state into live ANARI world/object handles. This is the bridge between TSD and actual rendering.
- **ImagePipeline**: Composable pipeline of render passes; the standard chain is ANARI render → composite (multi-device) → picking → AOV visualization.
- **CLI import routing**: Applications use a `-[format] file` pattern (e.g., `-obj mesh.obj`, `-vti vol.vti`) dispatched through `tsd_io` importers.

### Environment Variables

- `TSD_ANARI_LIBRARIES` — Colon-separated list of ANARI device libraries shown in the UI device selector.
