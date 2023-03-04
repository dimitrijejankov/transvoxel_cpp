#include "voxel_coordinates.hpp"
#include "implementation/rotation.hpp"

template <typename F>
inline Position<F> to_position_in_block(
    const Block<F>& block, const HighResolutionVoxelIndex& self)
{
    auto rotation = Rotation::for_side(self.cell.side);
    auto position_in_block = rotation.to_position_in_block(block.subdivisions, self);
    return position_in_block;
}

inline RegularVoxelIndex to_higher_res_neighbour_block_index(
    const HighResolutionVoxelIndex& self, std::size_t this_block_size)
{
    auto higher_res_block_size = static_cast<size_t>(this_block_size) * 2;
    auto cell = self.cell;
    auto delta = self.delta;
    auto rot = Rotation::for_side(cell.side);
    auto x = higher_res_block_size * (rot.uvw_base.x + rot.w.x)
        + delta.w * rot.w.x
        + (2 * cell.cell_u + delta.u) * rot.u.x
        + (2 * cell.cell_v + delta.v) * rot.v.x;
    auto y = higher_res_block_size * (rot.uvw_base.y + rot.w.y)
        + delta.w * rot.w.y
        + (2 * cell.cell_u + delta.u) * rot.u.y
        + (2 * cell.cell_v + delta.v) * rot.v.y;
    auto z = higher_res_block_size * (rot.uvw_base.z + rot.w.z)
        + delta.w * rot.w.z
        + (2 * cell.cell_u + delta.u) * rot.u.z
        + (2 * cell.cell_v + delta.v) * rot.v.z;
    return RegularVoxelIndex { x, y, z };
}