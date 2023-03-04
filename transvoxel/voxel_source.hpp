#pragma once

#include "density.hpp"
#include "structs.hpp"
#include "voxel_coordinates.hpp"
#include "implementation/rotation.hpp"

template <typename D>
class VoxelSource {
public:
    virtual D get_density(const RegularVoxelIndex& voxel_index) = 0;
    virtual D get_transition_density(const HighResolutionVoxelIndex& index) = 0;
    virtual ~VoxelSource() {}
};

template <typename C, typename D, typename SF>
class WorldMappingVoxelSource : public VoxelSource<D> {
public:
    WorldMappingVoxelSource(const SF &field, const Block<C> &block) : field(field), block(block) {}

    D get_density(const RegularVoxelIndex& voxel_index) override {
        C x = block.dims.base[0] + block.dims.size * Coordinate<C>::from_ratio(voxel_index.x, block.subdivisions);
        C y = block.dims.base[1] + block.dims.size * Coordinate<C>::from_ratio(voxel_index.y, block.subdivisions);
        C z = block.dims.base[2] + block.dims.size * Coordinate<C>::from_ratio(voxel_index.z, block.subdivisions);
        return field.get_density(x, y, z);
    }

    D get_transition_density(const HighResolutionVoxelIndex& index) override {
        auto rotation = Rotation::for_side(index.cell.side);
        auto position_in_block = rotation.to_position_in_block<C>(block.subdivisions, index);
        C x = block.dims.base[0] + block.dims.size * position_in_block.x;
        C y = block.dims.base[1] + block.dims.size * position_in_block.y;
        C z = block.dims.base[2] + block.dims.size * position_in_block.z;
        return field.get_density(x, y, z);
    }

private:

    SF field;
    Block<C> block;
};

template <typename D, typename F>
class VoxelSourceRef : public VoxelSource<D> {
public:
    VoxelSourceRef(F& source) : source(source) {}

    D get_density(const RegularVoxelIndex& voxel_index) override {
        return source.get_density(voxel_index);
    }

    D get_transition_density(const HighResolutionVoxelIndex& index) override {
        return source.get_transition_density(index);
    }

private:
    F& source;
};