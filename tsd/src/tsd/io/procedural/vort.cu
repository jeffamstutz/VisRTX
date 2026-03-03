// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "vort_cuda.h"
// tsd
#include "tsd/core/Logging.hpp"
// cuda
#include <cuda_runtime.h>
// std
#include <cmath>

// ---------------------------------------------------------------------------
// Device functions: lambda2 and Q-criterion (mirrors vort.h)
// ---------------------------------------------------------------------------

__device__ static double l2_d(double j00,
    double j01,
    double j02,
    double j10,
    double j11,
    double j12,
    double j20,
    double j21,
    double j22)
{
  // 6 unique entries of symmetric M = (J^2 + (J^T)^2) / 2
  const double m00 = j00 * j00 + j01 * j10 + j02 * j20;
  const double m11 = j10 * j01 + j11 * j11 + j12 * j21;
  const double m22 = j20 * j02 + j21 * j12 + j22 * j22;
  const double m01 = 0.5
      * (j00 * j01 + j01 * j11 + j02 * j21 + j00 * j10 + j10 * j11
          + j20 * j12);
  const double m02 = 0.5
      * (j00 * j02 + j01 * j12 + j02 * j22 + j00 * j20 + j10 * j21
          + j20 * j22);
  const double m12 = 0.5
      * (j10 * j02 + j11 * j12 + j12 * j22 + j01 * j20 + j11 * j21
          + j21 * j22);

  const double q = (m00 + m11 + m22) / 3.0;
  const double a = m00 - q, d = m11 - q, f = m22 - q;
  const double p1 = m01 * m01 + m02 * m02 + m12 * m12;
  if (p1 == 0.0) {
    // Diagonal — middle eigenvalue by inspection
    double e0 = a, e1 = d, e2 = f;
    if (e0 < e1) {
      double t = e0;
      e0 = e1;
      e1 = t;
    }
    if (e1 < e2) {
      double t = e1;
      e1 = e2;
      e2 = t;
    }
    if (e0 < e1) {
      double t = e0;
      e0 = e1;
      e1 = t;
    }
    (void)e0;
    (void)e2;
    return q + e1;
  }
  const double p = sqrt((a * a + d * d + f * f + 2.0 * p1) / 6.0);
  const double r = (a * (d * f - m12 * m12) - m01 * (m01 * f - m12 * m02)
                       + m02 * (m01 * m12 - d * m02))
      / (2.0 * p * p * p);
  const double phi = acos(fmax(-1.0, fmin(1.0, r))) / 3.0;
  const double e0 = q + 2.0 * p * cos(phi);
  const double e2 = q + 2.0 * p * cos(phi + 2.0943951023931953);
  return 3.0 * q - e0 - e2;
}

__device__ static double q_crit_d(double j00,
    double j01,
    double j02,
    double j10,
    double j11,
    double j12,
    double j20,
    double j21,
    double j22)
{
  return -0.5
      * (j00 * j00 + j11 * j11 + j22 * j22
          + 2.0 * (j01 * j10 + j02 * j20 + j12 * j21));
}

// ---------------------------------------------------------------------------
// grad3D_kernel — one thread per output element.
//
// axis 0 (x): i=ix, n=nx, stride s=1
// axis 1 (y): i=iy, n=ny, stride s=nx
// axis 2 (z): i=iz, n=nz, stride s=nx*ny
//
// Boundary: 1st-order one-sided.  Interior: 2nd-order central.
// ---------------------------------------------------------------------------

__global__ static void grad3D_kernel(const float *fc,
    double *gc,
    size_t nx,
    size_t ny,
    size_t nz,
    int axis,
    const double *c)
{
  const size_t len = nx * ny * nz;
  const size_t tid = (size_t)blockIdx.x * blockDim.x + threadIdx.x;
  if (tid >= len)
    return;

  const size_t slab = nx * ny;
  const size_t iz = tid / slab;
  const size_t iy = (tid % slab) / nx;
  const size_t ix = tid % nx;

  size_t i, n, s;
  if (axis == 0) {
    i = ix;
    n = nx;
    s = 1;
  } else if (axis == 1) {
    i = iy;
    n = ny;
    s = nx;
  } else {
    i = iz;
    n = nz;
    s = slab;
  }

  double val;
  if (i == 0)
    val = ((double)fc[tid + s] - (double)fc[tid]) / (c[1] - c[0]);
  else if (i == n - 1)
    val = ((double)fc[tid] - (double)fc[tid - s]) / (c[n - 1] - c[n - 2]);
  else
    val =
        ((double)fc[tid + s] - (double)fc[tid - s]) / (c[i + 1] - c[i - 1]);

  gc[tid] = val;
}

// ---------------------------------------------------------------------------
// vort_from_jacobians_kernel — one thread per voxel.
// ---------------------------------------------------------------------------

__global__ static void vort_from_jacobians_kernel(const float *u,
    const float *v,
    const float *w,
    const double *dux,
    const double *dvx,
    const double *dwx,
    const double *duy,
    const double *dvy,
    const double *dwy,
    const double *duz,
    const double *dvz,
    const double *dwz,
    float *vorticity,
    float *helicity,
    float *lambda2,
    float *qCriterion,
    size_t len)
{
  const size_t i = (size_t)blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= len)
    return;

  if (vorticity || helicity) {
    const double omx = dwy[i] - dvz[i];
    const double omy = duz[i] - dwx[i];
    const double omz = dvx[i] - duy[i];
    const double omag = sqrt(omx * omx + omy * omy + omz * omz);
    if (vorticity)
      vorticity[i] = (float)omag;
    if (helicity) {
      const double ui = u[i], vi = v[i], wi = w[i];
      const double h = fabs(omx * ui + omy * vi + omz * wi);
      const double vmag = sqrt(ui * ui + vi * vi + wi * wi);
      helicity[i] = (vmag > 0.0 && omag > 0.0)
          ? (float)(h / (2.0 * vmag * omag))
          : 0.0f;
    }
  }
  if (lambda2 || qCriterion) {
    if (lambda2)
      lambda2[i] = (float)(-fmin(l2_d(dux[i],
                                      duy[i],
                                      duz[i],
                                      dvx[i],
                                      dvy[i],
                                      dvz[i],
                                      dwx[i],
                                      dwy[i],
                                      dwz[i]),
          0.0));
    if (qCriterion)
      qCriterion[i] = (float)fmax(q_crit_d(dux[i],
                                      duy[i],
                                      duz[i],
                                      dvx[i],
                                      dvy[i],
                                      dvz[i],
                                      dwx[i],
                                      dwy[i],
                                      dwz[i]),
          0.0);
  }
}

// ---------------------------------------------------------------------------
// vort_cuda — host entry point
// ---------------------------------------------------------------------------

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
    size_t nz)
{
  if (!vorticity && !helicity && !lambda2 && !qCriterion)
    return;

  const size_t len = nx * ny * nz;
  const int BLOCK = 256;
  const int grid = (int)((len + BLOCK - 1) / BLOCK);

  tsd::core::logStatus(
      "[vort_cuda] computing on %zux%zux%zu grid (%zu voxels)...",
      nx,
      ny,
      nz,
      len);

  // Device pointers — all null-initialized so cudaFree(nullptr) is safe on
  // any early-exit path.
  float *d_u{}, *d_v{}, *d_w{};
  double *d_x{}, *d_y{}, *d_z{};
  double *d_dux{}, *d_dvx{}, *d_dwx{};
  double *d_duy{}, *d_dvy{}, *d_dwy{};
  double *d_duz{}, *d_dvz{}, *d_dwz{};
  float *d_vort{}, *d_hel{}, *d_l2{}, *d_qc{};

  bool ok = true;
  auto check = [&](cudaError_t e) {
    if (e != cudaSuccess && ok) {
      tsd::core::logError(
          "[vort_cuda] CUDA error: %s", cudaGetErrorString(e));
      ok = false;
    }
  };

  // Allocate and upload velocity fields
  check(cudaMalloc(&d_u, len * sizeof(float)));
  check(cudaMalloc(&d_v, len * sizeof(float)));
  check(cudaMalloc(&d_w, len * sizeof(float)));
  if (ok) {
    check(cudaMemcpy(d_u, u, len * sizeof(float), cudaMemcpyHostToDevice));
    check(cudaMemcpy(d_v, v, len * sizeof(float), cudaMemcpyHostToDevice));
    check(cudaMemcpy(d_w, w, len * sizeof(float), cudaMemcpyHostToDevice));
  }

  // Allocate and upload coordinate arrays
  check(cudaMalloc(&d_x, nx * sizeof(double)));
  check(cudaMalloc(&d_y, ny * sizeof(double)));
  check(cudaMalloc(&d_z, nz * sizeof(double)));
  if (ok) {
    check(cudaMemcpy(d_x, x_, nx * sizeof(double), cudaMemcpyHostToDevice));
    check(cudaMemcpy(d_y, y_, ny * sizeof(double), cudaMemcpyHostToDevice));
    check(cudaMemcpy(d_z, z_, nz * sizeof(double), cudaMemcpyHostToDevice));
  }

  // Allocate Jacobian scratch (9 × len doubles)
  check(cudaMalloc(&d_dux, len * sizeof(double)));
  check(cudaMalloc(&d_dvx, len * sizeof(double)));
  check(cudaMalloc(&d_dwx, len * sizeof(double)));
  check(cudaMalloc(&d_duy, len * sizeof(double)));
  check(cudaMalloc(&d_dvy, len * sizeof(double)));
  check(cudaMalloc(&d_dwy, len * sizeof(double)));
  check(cudaMalloc(&d_duz, len * sizeof(double)));
  check(cudaMalloc(&d_dvz, len * sizeof(double)));
  check(cudaMalloc(&d_dwz, len * sizeof(double)));

  // Allocate output buffers
  if (vorticity)
    check(cudaMalloc(&d_vort, len * sizeof(float)));
  if (helicity)
    check(cudaMalloc(&d_hel, len * sizeof(float)));
  if (lambda2)
    check(cudaMalloc(&d_l2, len * sizeof(float)));
  if (qCriterion)
    check(cudaMalloc(&d_qc, len * sizeof(float)));

  if (ok) {
    // Compute all 9 Jacobian components (u,v,w) × (x,y,z)
    grad3D_kernel<<<grid, BLOCK>>>(d_u, d_dux, nx, ny, nz, 0, d_x);
    grad3D_kernel<<<grid, BLOCK>>>(d_v, d_dvx, nx, ny, nz, 0, d_x);
    grad3D_kernel<<<grid, BLOCK>>>(d_w, d_dwx, nx, ny, nz, 0, d_x);
    grad3D_kernel<<<grid, BLOCK>>>(d_u, d_duy, nx, ny, nz, 1, d_y);
    grad3D_kernel<<<grid, BLOCK>>>(d_v, d_dvy, nx, ny, nz, 1, d_y);
    grad3D_kernel<<<grid, BLOCK>>>(d_w, d_dwy, nx, ny, nz, 1, d_y);
    grad3D_kernel<<<grid, BLOCK>>>(d_u, d_duz, nx, ny, nz, 2, d_z);
    grad3D_kernel<<<grid, BLOCK>>>(d_v, d_dvz, nx, ny, nz, 2, d_z);
    grad3D_kernel<<<grid, BLOCK>>>(d_w, d_dwz, nx, ny, nz, 2, d_z);

    // Compute vortical quantities from the Jacobian
    vort_from_jacobians_kernel<<<grid, BLOCK>>>(d_u,
        d_v,
        d_w,
        d_dux,
        d_dvx,
        d_dwx,
        d_duy,
        d_dvy,
        d_dwy,
        d_duz,
        d_dvz,
        d_dwz,
        d_vort,
        d_hel,
        d_l2,
        d_qc,
        len);

    check(cudaDeviceSynchronize());
  }

  if (ok) {
    if (vorticity)
      check(cudaMemcpy(
          vorticity, d_vort, len * sizeof(float), cudaMemcpyDeviceToHost));
    if (helicity)
      check(cudaMemcpy(
          helicity, d_hel, len * sizeof(float), cudaMemcpyDeviceToHost));
    if (lambda2)
      check(cudaMemcpy(
          lambda2, d_l2, len * sizeof(float), cudaMemcpyDeviceToHost));
    if (qCriterion)
      check(cudaMemcpy(
          qCriterion, d_qc, len * sizeof(float), cudaMemcpyDeviceToHost));
  }

  cudaFree(d_u);
  cudaFree(d_v);
  cudaFree(d_w);
  cudaFree(d_x);
  cudaFree(d_y);
  cudaFree(d_z);
  cudaFree(d_dux);
  cudaFree(d_dvx);
  cudaFree(d_dwx);
  cudaFree(d_duy);
  cudaFree(d_dvy);
  cudaFree(d_dwy);
  cudaFree(d_duz);
  cudaFree(d_dvz);
  cudaFree(d_dwz);
  cudaFree(d_vort);
  cudaFree(d_hel);
  cudaFree(d_l2);
  cudaFree(d_qc);
}
