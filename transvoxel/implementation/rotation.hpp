#pragma once

#include <cstdint>
#include <iostream>
#include <tuple>

#include "../transition_sides.hpp"
#include "../structs.hpp"
#include "../voxel_coordinates.hpp"
#include "../density.hpp"

struct XYZ {

    XYZ(std::tuple<int64_t , int64_t , int64_t > xyz) :
        x(std::get<0>(xyz)),
        y(std::get<1>(xyz)),
        z(std::get<2>(xyz)) {}

    int64_t  x;
    int64_t  y;
    int64_t  z;
};


/**
Rotation information for one of the 6 [TransitionSide]
*/
struct Rotation {
    /**
    Gives the position of this voxel relative to the block (each coordinate ranging from 0 to 1)
    */
    template <typename F>
    Position<F> to_position_in_block(size_t block_size, const HighResolutionVoxelIndex& voxel_index) const {
        const auto cell_index = voxel_index.cell;
        const auto delta = voxel_index.delta;

        // We multiply by 2 most things to divide in the end, in an attempt to reduce floating point operations (maybe need to measure if this is gaining us anything)
        const auto x = uvw_base.x * 2 * static_cast<int64_t >(block_size)
            + u.x * (2 * static_cast<int64_t >(cell_index.cell_u) + delta.u)
            + v.x * (2 * static_cast<int64_t >(cell_index.cell_v) + delta.v)
            + w.x * delta.w;
        const auto y = uvw_base.y * 2 * static_cast<int64_t >(block_size)
            + u.y * (2 * static_cast<int64_t >(cell_index.cell_u) + delta.u)
            + v.y * (2 * static_cast<int64_t >(cell_index.cell_v) + delta.v)
            + w.y * delta.w;
        const auto z = uvw_base.z * 2 * static_cast<int64_t >(block_size)
            + u.z * (2 * static_cast<int64_t >(cell_index.cell_u) + delta.u)
            + v.z * (2 * static_cast<int64_t >(cell_index.cell_v) + delta.v)
            + w.z * delta.w;

        return { Coordinate<F>::half(x) * Coordinate<F>::from_ratio(1, block_size),
                 Coordinate<F>::half(y) * Coordinate<F>::from_ratio(1, block_size),
                 Coordinate<F>::half(z) * Coordinate<F>::from_ratio(1, block_size) };
    }

    RegularVoxelIndex to_regular_voxel_index(size_t block_size,
                                             const TransitionCellIndex& cell_index,
                                             size_t face_u, size_t face_v) const {
        const auto x = uvw_base.x * static_cast<int64_t >(block_size)
            + u.x * static_cast<int64_t >(cell_index.cell_u + face_u)
            + v.x * static_cast<int64_t >(cell_index.cell_v + face_v);
        const auto y = uvw_base.y * static_cast<int64_t >(block_size)
            + u.y * static_cast<int64_t >(cell_index.cell_u + face_u)
            + v.y * static_cast<int64_t >(cell_index.cell_v + face_v);
        const auto z = uvw_base.z * static_cast<int64_t >(block_size)
            + u.z * static_cast<int64_t >(cell_index.cell_u + face_u)
            + v.z * static_cast<int64_t >(cell_index.cell_v + face_v);

        return { x, y, z };
    }

    static const Rotation& for_side(TransitionSide side);

    static const Rotation& create_default();

    Rotation(
        TransitionSide side,
        std::tuple<int64_t , int64_t , int64_t > uvw_base,
        std::tuple<int64_t , int64_t , int64_t > u,
        std::tuple<int64_t , int64_t , int64_t > v,
        std::tuple<int64_t , int64_t , int64_t > w,
        std::tuple<int64_t , int64_t , int64_t > /*xyz_base*/,
        std::tuple<int64_t , int64_t , int64_t > x,
        std::tuple<int64_t , int64_t , int64_t > y,
        std::tuple<int64_t , int64_t , int64_t > z
    )
        : side{side},
        uvw_base{XYZ(uvw_base)},
        u{XYZ(u)},
        v{XYZ(v)},
        w{XYZ(w)},
        plus_x_as_uvw{HighResolutionVoxelDelta{x}},
        plus_y_as_uvw{HighResolutionVoxelDelta{y}},
        plus_z_as_uvw{HighResolutionVoxelDelta{z}}
    {}

    /**
    The side is just stored along here for convenience
    */
    TransitionSide side;
    /**
    Where the origin of the UVW system is, in the XYZ system. It is used both for cells and blocks. 0 is at the lowest of the cell or block, 1 at the highest
    */
    XYZ uvw_base;
    /**
    The direction of the U unit vector, in the XYZ system. Components can be -1 0 or 1
    */
    XYZ u;
    /**
    The direction of the V unit vector, in the XYZ system. Components can be -1 0 or 1
    */
    XYZ v;
    /**
    The direction of the W unit vector, in the XYZ system. Components can be -1 0 or 1
    */
    XYZ w;
    /**
    The direction of the X unit vector, in the UVW system. Components can be -1 0 or 1
    */
    HighResolutionVoxelDelta plus_x_as_uvw;
    /**
    The direction of the Y unit vector, in the UVW system. Components can be -1 0 or 1
    */
    HighResolutionVoxelDelta plus_y_as_uvw;
    /**
    The direction of the Z unit vector, in the UVW system. Components can be -1 0 or 1
    */
    HighResolutionVoxelDelta plus_z_as_uvw;
};