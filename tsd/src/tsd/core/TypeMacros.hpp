// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define TSD_DEFAULT_MOVEABLE(TYPE)                                             \
  TYPE(TYPE &&) = default;                                                     \
  TYPE &operator=(TYPE &&) = default;

#define TSD_DEFAULT_COPYABLE(TYPE)                                             \
  TYPE(const TYPE &) = default;                                                \
  TYPE &operator=(const TYPE &) = default;

#define TSD_NOT_MOVEABLE(TYPE)                                                 \
  TYPE(TYPE &&) = delete;                                                      \
  TYPE &operator=(TYPE &&) = delete;

#define TSD_NOT_COPYABLE(TYPE)                                                 \
  TYPE(const TYPE &) = delete;                                                 \
  TYPE &operator=(const TYPE &) = delete;
