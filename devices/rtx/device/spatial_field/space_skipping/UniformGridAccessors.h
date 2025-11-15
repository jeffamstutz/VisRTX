/*
 * Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "gpu/gpu_decl.h"
#include "gpu/gpu_objects.h"

// cuda
#include <texture_types.h>

// nanovdb
#include <nanovdb/NanoVDB.h>

namespace visrtx {

// Voxel accessor classes for UniformGrid space-skipping grid construction.
// These read raw voxel values by integer coordinate, bypassing world-space
// coordinate transforms entirely.

template <typename T>
class SpatialFieldAccessor
{
};

template <>
class SpatialFieldAccessor<cudaTextureObject_t>
{
 public:
  VISRTX_DEVICE SpatialFieldAccessor(const SpatialFieldGPUData &sf)
      : m_texObj(sf.data.structuredRegular.texObj)
  {}

  VISRTX_DEVICE float operator()(int ix, int iy, int iz)
  {
    return tex3D<float>(m_texObj, ix + 0.5f, iy + 0.5f, iz + 0.5f);
  }

 private:
  cudaTextureObject_t m_texObj;
};

template <typename T>
class SpatialFieldAccessor<nanovdb::Grid<nanovdb::NanoTree<T>>>
{
  using GridType = typename nanovdb::Grid<nanovdb::NanoTree<T>>;
  using AccessorType = typename GridType::AccessorType;

 public:
  VISRTX_DEVICE SpatialFieldAccessor(const SpatialFieldGPUData &sf)
      : m_grid(
            reinterpret_cast<const GridType *>(sf.data.nvdbRegular.gridData)),
        m_accessor(m_grid->getAccessor()),
        m_indexMin(m_grid->indexBBox().min())
  {}

  VISRTX_DEVICE float operator()(int ix, int iy, int iz)
  {
    return m_accessor.getValue(nanovdb::Coord(
        ix + m_indexMin[0], iy + m_indexMin[1], iz + m_indexMin[2]));
  }

 private:
  const GridType *m_grid;
  AccessorType m_accessor;
  nanovdb::Coord m_indexMin;
};

template <typename T>
using NvdbSpatialFieldAccessor =
    SpatialFieldAccessor<nanovdb::Grid<nanovdb::NanoTree<T>>>;

class StructuredRectilinearAccessor
{
 public:
  VISRTX_DEVICE StructuredRectilinearAccessor(const SpatialFieldGPUData &sf)
      : m_texObj(sf.data.structuredRectilinear.texObj)
  {}

  VISRTX_DEVICE float operator()(int ix, int iy, int iz)
  {
    return tex3D<float>(m_texObj, ix + 0.5f, iy + 0.5f, iz + 0.5f);
  }

 private:
  cudaTextureObject_t m_texObj;
};

template <typename T>
class NvdbRectilinearSpatialFieldAccessor
{
  using GridType = typename nanovdb::Grid<nanovdb::NanoTree<T>>;
  using AccessorType = typename GridType::AccessorType;

 public:
  VISRTX_DEVICE NvdbRectilinearSpatialFieldAccessor(
      const SpatialFieldGPUData &sf)
      : m_grid(reinterpret_cast<const GridType *>(
            sf.data.nvdbRectilinear.gridData)),
        m_accessor(m_grid->getAccessor()),
        m_indexMin(m_grid->indexBBox().min())
  {}

  VISRTX_DEVICE float operator()(int ix, int iy, int iz)
  {
    return m_accessor.getValue(nanovdb::Coord(
        ix + m_indexMin[0], iy + m_indexMin[1], iz + m_indexMin[2]));
  }

 private:
  const GridType *m_grid;
  AccessorType m_accessor;
  nanovdb::Coord m_indexMin;
};

} // namespace visrtx
