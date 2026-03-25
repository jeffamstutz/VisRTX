// Copyright 2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#ifdef __CUDACC__
#define TSD_DEVICE_FCN __device__
#define TSD_DEVICE_FCN_INLINE __forceinline__ __device__
#define TSD_HOST_DEVICE_FCN __host__ __device__
#else
#define TSD_DEVICE_FCN
#define TSD_DEVICE_FCN_INLINE inline
#define TSD_HOST_DEVICE_FCN
#endif
