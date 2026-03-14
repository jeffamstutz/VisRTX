## Viskores Demo (`tsdDemoViskores`)

This demo integrates a Viskores execution graph with the TSD interactive UI.

It adds graph-specific windows (`Node Editor`, `Node Info`) and updates the
graph each frame, while showing results in a standard TSD viewport/editor
layout.

By default (`TSD_DEVICE_PASSTHROUGH=1` in source), it uses `anari_tsd` as a
pass-through ANARI device and mirrors graph output into the live TSD scene.

### Build Notes

- Not built by default.
- Enable with `-DBUILD_VISKORES_DEMO=ON`.
- Requires `viskores_graph` (`find_package(viskores_graph 1.1.0 REQUIRED)`).

### Run

```bash
./tsdDemoViskores
```
