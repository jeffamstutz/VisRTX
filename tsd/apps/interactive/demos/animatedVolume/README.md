## Animated Volume Demo (`tsdDemoAnimatedVolume`)

This demo runs a CUDA Jacobi 3D solver on a structured volume and visualizes
the evolving scalar field in real time.

It wires solver output to a TSD `structuredRegular` spatial field and a
`transferFunction1D` volume, so transfer-function and isosurface tools can be
used while the simulation runs.

### Build Notes

- Requires `TSD_USE_CUDA=ON`.
- Built only when CUDA support is enabled.

### Run

```bash
./tsdDemoAnimatedVolume
```

### In-App Controls

- Solver playback: `reset`, `play/stop`, `iterate`.
- Grid sizing: cubic or per-axis dimensions.
- Iterations per cycle.
- Compute/data path: GPU array interop on/off.
- Optional automatic transfer-function update.
- Optional timestep export to `.raw` volumes.
