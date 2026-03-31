// Copyright 2024-2026 NVIDIA Corporation
// SPDX-License-Identifier: Apache-2.0

#include "ImagePass.h"
// tsd_algorithms
#include "tsd/algorithms/cpu/convertColorBuffer.hpp"
#ifdef TSD_ALGORITHMS_HAS_CUDA
#include "tsd/algorithms/cuda/convertColorBuffer.hpp"
#endif
// std
#include <algorithm>
#include <cstring>

#if USE_CUDA
#include "cuda_runtime.h"
#endif

namespace tsd::rendering {

ImagePass::ImagePass() = default;

ImagePass::~ImagePass() = default;

void ImagePass::setEnabled(bool enabled)
{
  m_enabled = enabled;
}

bool ImagePass::isEnabled() const
{
  return m_enabled;
}

const char *ImagePass::name() const
{
  return "ImagePass";
}

void ImagePass::updateSize()
{
  // no-up
}

tsd::math::uint2 ImagePass::getDimensions() const
{
  return m_size;
}

void ImagePass::setDimensions(uint32_t width, uint32_t height)
{
  if (m_size.x == width && m_size.y == height)
    return;

  m_size.x = width;
  m_size.y = height;

  updateSize();
}

// Utility functions //////////////////////////////////////////////////////////

namespace detail {

void *allocate_(size_t numBytes)
{
#ifdef ENABLE_CUDA
  void *ptr = nullptr;
  cudaMallocManaged(&ptr, numBytes);
  return ptr;
#else
  return std::malloc(numBytes);
#endif
}

void free_(void *ptr)
{
#ifdef ENABLE_CUDA
  cudaFree(ptr);
#else
  std::free(ptr);
#endif
}

void memcpy_(void *dst, const void *src, size_t numBytes)
{
#ifdef ENABLE_CUDA
  cudaMemcpy(dst, src, numBytes, cudaMemcpyDefault);
#else
  std::memcpy(dst, src, numBytes);
#endif
}

void convertFloatColorBuffer_(
    ComputeStream stream, const float *v, uint8_t *out, size_t totalSize)
{
#ifdef TSD_ALGORITHMS_HAS_CUDA
  if (stream) {
    tsd::algorithms::cuda::convertFloatToUint8(stream, v, out, totalSize);
    return;
  }
#endif
  tsd::algorithms::cpu::convertFloatToUint8(v, out, totalSize);
}

} // namespace detail

} // namespace tsd::rendering
