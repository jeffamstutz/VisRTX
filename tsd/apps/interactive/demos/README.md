## Interactive Demos

This directory contains focused interactive applications that demonstrate
specific TSD/ANARI workflows.

| Demo | Executable | Directory | Summary | Build Requirements |
| --- | --- | --- | --- | --- |
| [Animated Particles](animatedParticles/README.md) | `tsdDemoAnimatedParticles` | [animatedParticles/](animatedParticles/) | CUDA particle simulation with moving attractors, rendered via TSD geometry and colormapping. | `TSD_USE_CUDA=ON` |
| [Animated Volume](animatedVolume/README.md) | `tsdDemoAnimatedVolume` | [animatedVolume/](animatedVolume/) | CUDA Jacobi 3D volume solver with live transfer-function/isosurface visualization. | `TSD_USE_CUDA=ON` |
| [Array Instancing](arrayInstancing/README.md) | `tsdDemoArrayInstancing` | [arrayInstancing/](arrayInstancing/) | Stress/demo scene for large instancing, transform arrays, and per-instance attributes. | Interactive apps enabled |
| [Manual Accumulation Reset](manualAccumulationReset/README.md) | `tsdDemoAccumulationReset` | [manualAccumulationReset/](manualAccumulationReset/) | Demonstrates manual accumulation reset control through custom frame parameters. | Interactive apps enabled |
| [Viskores](viskores/README.md) | `tsdDemoViskores` | [viskores/](viskores/) | Viskores graph editor integrated with TSD UI and ANARI device pass-through. | `BUILD_VISKORES_DEMO=ON`, `viskores_graph` |
