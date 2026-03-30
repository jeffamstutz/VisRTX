# TSD Style Guide

This document covers TSD-specific coding conventions. All general C++ and CUDA
rules from [`../STYLEGUIDE.md`](../STYLEGUIDE.md) apply here as well.

---

## Prefer TSD Primitives Over Standard Alternatives

Before reaching for a standard container or writing a data structure from
scratch, check whether a TSD primitive already fits:

| Need | Use instead of | TSD type |
|---|---|---|
| Ordered key→value map (small, stable keys) | `std::map` / `std::unordered_map` | `FlatMap<K,V>` (`tsd/core/FlatMap.hpp`) |
| Linked list or parent–child tree | `std::list` / hand-rolled | `Forest<T>` / `ForestNode<T>` (`tsd/core/Forest.hpp`) |
| Stable-handle object pool | `std::vector` + index | `ObjectPool<T>` + `ObjectPoolRef<T>` (`tsd/core/ObjectPool.hpp`) |
| ANARI-typed parameter value | `void *` / `std::any` | `tsd::core::Any` (`tsd/core/Any.hpp`) |

---

## Scene Mutation and Notification

- Subclass `BaseUpdateDelegate` for any consumer that needs to react to scene
  mutations (renderer synchronization, network replication, UI refresh).
- Use `MultiUpdateDelegate` to fan notifications out to multiple consumers
  without writing fan-out logic yourself.
- Never bypass the delegate system by mutating scene objects and then calling
  renderer internals directly — that breaks the synchronization contract.

---

## Parameter Builder Pattern

Chain `Parameter` setters rather than setting each property separately:

```cpp
p->setDescription("Sphere radius")
  .setValue(0.5f)
  .setMin(0.f)
  .setMax(10.f);
```

---

## Library Layering

Respect the dependency order — never introduce an upward dependency:

```
tsd_core  →  tsd_scene  →  tsd_io  →  tsd_rendering  →  tsd_app
```

Optional libraries (`tsd_ui_imgui`, `tsd_mpi`, `tsd_network`, `tsd_lua`) may
depend on any layer but must remain optional (CMake-gated).

---

## GPU Computation (`TSD_USE_CUDA`)

Prefer Thrust algorithms over hand-written `__global__` kernels:

```cpp
// Prefer:
thrust::transform(thrust::cuda::par.on(stream), begin, end, out, op);

// Over:
myCustomKernel<<<grid, block, 0, stream>>>(begin, end, out);
```

Write a custom kernel only when no suitable Thrust primitive exists and the
algorithm cannot be composed from existing ones.
