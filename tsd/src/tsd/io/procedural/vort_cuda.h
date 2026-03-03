// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstddef>

// GPU-accelerated vortical field computation.
// Same signature and semantics as vort() in vort.h, but runs on the GPU.
// All pointers are host pointers; data transfer is managed internally.
// Any output pointer may be null; null outputs are skipped.
void vort_cuda(const float *u,
    const float *v,
    const float *w,
    const double *x_,
    const double *y_,
    const double *z_,
    float *vorticity,
    float *helicity,
    float *lambda2,
    float *qCriterion,
    size_t nx,
    size_t ny,
    size_t nz);
