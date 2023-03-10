#pragma once

#include <cstddef>  // for std::size_t
#include <cstdint>
#include <utility>  // for std::pair
#include <cmath>    // for std::round
#include <tuple>

#include "transition_sides.hpp"
#include "structs.hpp"

struct Rotation;

struct RegularCellIndex {
    std::size_t x;
    std::size_t y;
    std::size_t z;
};

struct RegularVoxelDelta {
    int64_t  x;
    int64_t  y;
    int64_t  z;
};

struct RegularVoxelIndex {
    int64_t  x;
    int64_t  y;
    int64_t  z;
};

inline RegularVoxelIndex operator+(const RegularCellIndex& lhs, const RegularVoxelDelta& rhs) {
    return RegularVoxelIndex {
        static_cast<int64_t >(lhs.x) + rhs.x,
        static_cast<int64_t >(lhs.y) + rhs.y,
        static_cast<int64_t >(lhs.z) + rhs.z
    };
}

inline RegularVoxelIndex operator+(const RegularVoxelIndex& lhs, const RegularVoxelDelta& rhs) {
    return RegularVoxelIndex {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z
    };
}


struct TransitionCellIndex {
    TransitionSide side;
    std::size_t cell_u;
    std::size_t cell_v;
};

inline TransitionCellIndex from_transition_side(TransitionSide side, std::size_t cell_u, std::size_t cell_v) {
    return TransitionCellIndex { side, cell_u, cell_v };
}

struct HighResolutionVoxelDelta {

    HighResolutionVoxelDelta() = default;

    HighResolutionVoxelDelta(const std::tuple<int64_t , int64_t , int64_t > &uvw) :
        u(std::get<0>(uvw)),
        v(std::get<1>(uvw)),
        w(std::get<2>(uvw)) {}

    /// U. From -1 to 3 (included). 0 to 2 are within the cell. -1 and 3 extend out, for gradient computations
    int64_t  u;
    /// V. From -1 to 3 (included). 0 to 2 are within the cell. -1 and 3 extend out, for gradient computations
    int64_t  v;
    /// W. From -1 to 1. 0 is on the face, 1 is within the cell, -1 is outside the cell
    int64_t  w;
};

struct HighResolutionVoxelIndex {
    TransitionCellIndex cell;
    HighResolutionVoxelDelta delta;
};

inline HighResolutionVoxelIndex from_transition_side(
    TransitionSide side, std::size_t cell_u, std::size_t cell_v, int64_t  u, int64_t  v, int64_t  w)
{
    return HighResolutionVoxelIndex {
        .cell = from_transition_side(side, cell_u, cell_v),
        .delta = HighResolutionVoxelDelta({ u, v, w })
    };
}

inline bool on_regular_grid(const HighResolutionVoxelIndex& self) {
    bool du_on_regular_grid = (self.delta.u % 2) == 0;
    bool dv_on_regular_grid = (self.delta.v % 2) == 0;
    bool dw_on_regular_grid = self.delta.w == 0;
    return du_on_regular_grid && dv_on_regular_grid && dw_on_regular_grid;
}

RegularVoxelIndex as_regular_index(const HighResolutionVoxelIndex& voxelIndex, const Rotation& rotation, size_t block_subdivisions);

inline Position<float> to_position_in_block(const Block<float>& block, const HighResolutionVoxelIndex& self);

inline RegularVoxelIndex to_higher_res_neighbour_block_index(
    const HighResolutionVoxelIndex& self, std::size_t this_block_size);

inline HighResolutionVoxelDelta from_high_res(uint64_t  u, uint64_t  v, uint64_t  w) {
    return HighResolutionVoxelDelta({ u, v, w });
}

inline HighResolutionVoxelIndex operator+(
    const TransitionCellIndex& lhs, const HighResolutionVoxelDelta& rhs)
{
    return HighResolutionVoxelIndex { lhs, rhs };
}

inline HighResolutionVoxelIndex operator+(
    const HighResolutionVoxelIndex& lhs, const HighResolutionVoxelDelta& rhs)
{
    return HighResolutionVoxelIndex {
        lhs.cell,
        from_high_res(
            lhs.delta.u + rhs.u,
            lhs.delta.v + rhs.v,
            lhs.delta.w + rhs.w
        )
    };
}

inline HighResolutionVoxelIndex operator-(
    const HighResolutionVoxelIndex& lhs, const HighResolutionVoxelDelta& rhs)
{
    return HighResolutionVoxelIndex {
        lhs.cell,
        from_high_res(
            lhs.delta.u - rhs.u,
            lhs.delta.v - rhs.v,
            lhs.delta.w - rhs.w
        )
    };
}