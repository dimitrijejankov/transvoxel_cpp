#pragma once

#include "rotation.hpp"
#include "transvoxel.h"
#include "aux_tables.hpp"
#include "table_wrappers.hpp"
#include <cstddef>
#include <cstdint>

template <typename F, typename D>
struct GridPoint {
    Position<F> position;
    std::tuple<F, F, F> gradient;
    D density;
};

struct SharedVertexIndices {
    std::vector<size_t> regular;
    std::vector<size_t> transition;
    size_t block_size;

    SharedVertexIndices(size_t block_size) :
        regular(4 * block_size * block_size * block_size, 0),
        transition(10 * 6 * block_size * block_size, 0),
        block_size(block_size) {}

    size_t get_regular(size_t cell_x, size_t cell_y, size_t cell_z, size_t reuse_index) const {
        size_t storage_index = cell_x
            + block_size * cell_y
            + block_size * block_size * cell_z
            + block_size * block_size * block_size * reuse_index;
        return regular[storage_index];
    }

    void put_regular(size_t index, size_t cell_x, size_t cell_y, size_t cell_z, size_t reuse_index) {
        size_t storage_index = cell_x
            + block_size * cell_y
            + block_size * block_size * cell_z
            + block_size * block_size * block_size * reuse_index;
        regular[storage_index] = index;
    }
    size_t get_transition(const TransitionCellIndex& cell, const TransitionReuseIndex& reuse_index) const {
        size_t storage_index = static_cast<uint8_t>(cell.side)
            + 6 * cell.cell_u
            + 6 * block_size * cell.cell_v
            + 6 * block_size * block_size * reuse_index.index;
        return transition[storage_index];
    }
    void put_transition(size_t index, const TransitionCellIndex& cell, TransitionReuseIndex reuse_index) {
        size_t storage_index = static_cast<uint8_t>(cell.side) + 6 * cell.cell_u
            + 6 * block_size * cell.cell_v
            + 6 * block_size * block_size * reuse_index.index;
        transition[storage_index] = index;
    }
};

inline bool can_shrink(size_t xi, size_t yi, size_t zi, size_t subdivisions,
                const TransitionSides &transition_sides) {
    bool dont_shrink =
        ((xi == 0) && !transition_sides.test(static_cast<size_t>(TransitionSide::LowX))) ||
        ((xi == static_cast<size_t>(subdivisions)) &&
         !transition_sides.test(static_cast<size_t>(TransitionSide::HighX))) ||
        ((yi == 0) && !transition_sides.test(static_cast<size_t>(TransitionSide::LowY))) ||
        ((yi == static_cast<size_t>(subdivisions)) &&
         !transition_sides.test(static_cast<size_t>(TransitionSide::HighY))) ||
        ((zi == 0) && !transition_sides.test(static_cast<size_t>(TransitionSide::LowZ))) ||
        ((zi == static_cast<size_t>(subdivisions)) &&
         !transition_sides.test(static_cast<size_t>(TransitionSide::HighZ)));
    return !dont_shrink;
}

template <typename F, typename D>
void _shrink_if_needed(F &x, F &y, F &z,
                      size_t xi, size_t yi, size_t zi, F cell_size,
                      size_t subdivisions, const TransitionSides &transition_sides) {
    auto shrink = Density<D>::shrink_factor() * cell_size;
    if (can_shrink(xi, yi, zi, subdivisions, transition_sides)) {
        if ((xi == 0) && (transition_sides.test(static_cast<size_t>(TransitionSide::LowX)))) {
            x = x + shrink;
        } else if ((xi == static_cast<size_t>(subdivisions)) &&
                   (transition_sides.test(static_cast<size_t>(TransitionSide::HighX)))) {
            x = x - shrink;
        }
        if ((yi == 0) && (transition_sides.test(static_cast<size_t>(TransitionSide::LowY)))) {
            y = y + shrink;
        } else if ((yi == static_cast<size_t>(subdivisions)) &&
                   (transition_sides.test(static_cast<size_t>(TransitionSide::HighY)))) {
            y = y - shrink;
        }
        if ((zi == 0) && (transition_sides.test(static_cast<size_t>(TransitionSide::LowZ)))) {
            z = z + shrink;
        } else if ((zi == static_cast<size_t>(subdivisions)) &&
                   (transition_sides.test(static_cast<size_t>(TransitionSide::HighZ)))) {
            z = z - shrink;
        }
    }
}

template<typename F, typename D, typename S>
struct Extractor {
    S density_source;
    Block<F> block;
    D threshold;
    TransitionSides transition_sides;
    size_t vertices;
    std::vector<F> vertices_positions;
    std::vector<F> vertices_normals;
    std::vector<size_t> tri_indices;
    SharedVertexIndices shared_storage;
    Rotation current_rotation;


    Extractor(S density_source, const Block<F> &block, D threshold, TransitionSides transition_sides)
        : density_source(density_source),
        block(block),
        threshold(threshold),
        transition_sides(transition_sides),
        vertices(0),
        vertices_positions(),
        vertices_normals(),
        tri_indices(),
        shared_storage(block.subdivisions),
        current_rotation(Rotation::create_default())
    {}

    Mesh<F> extract() {
        extract_regular_cells();
        extract_transition_cells();
        return output_mesh();
    }

    Mesh<F> output_mesh() {
        return Mesh<F>(
            std::move(vertices_positions),
            std::move(vertices_normals),
            std::move(tri_indices));
    }

    void extract_regular_cells() {
        for (size_t cell_x = 0; cell_x < block.subdivisions; ++cell_x) {
            for (size_t cell_y = 0; cell_y < block.subdivisions; ++cell_y) {
                for (size_t cell_z = 0; cell_z < block.subdivisions; ++cell_z) {
                    RegularCellIndex cell_index { cell_x, cell_y, cell_z };
                    extract_regular_cell(cell_index);
                }
            }
        }
    }

    void extract_regular_cell(const RegularCellIndex& cell_index) {
        const uint8_t case_number = regular_cell_case(cell_index);
        const uint8_t cell_class = regularCellClass[case_number];
        const RegularCellData& triangulation_info = regularCellData[cell_class];
        const auto& vertices_data = regularVertexData[case_number];
        std::array<size_t, 12> cell_vertices_indices {};
        for (size_t i = 0; i < VERTEX_DATA_LENGTH; ++i) {
            if (i >= triangulation_info.GetVertexCount()) {
                break;
            }
            cell_vertices_indices[i] = regular_vertex(cell_index, RegularVertexData{.data=vertices_data[i]});
        }
        for (size_t t = 0; t < triangulation_info.GetTriangleCount(); ++t) {
            const size_t v1_index_in_cell = triangulation_info.vertexIndex[3 * t];
            const size_t v2_index_in_cell = triangulation_info.vertexIndex[3 * t + 1];
            const size_t v3_index_in_cell = triangulation_info.vertexIndex[3 * t + 2];
            const size_t global_index_1 = cell_vertices_indices[v1_index_in_cell];
            const size_t global_index_2 = cell_vertices_indices[v2_index_in_cell];
            const size_t global_index_3 = cell_vertices_indices[v3_index_in_cell];
            tri_indices.push_back(global_index_1);
            tri_indices.push_back(global_index_2);
            tri_indices.push_back(global_index_3);
        }
    }
    size_t regular_cell_case(const RegularCellIndex& cell_index) {
        size_t case_number = 0;
        for (size_t i = 0; i < REGULAR_CELL_VOXELS.size(); ++i) {
            const auto& deltas = REGULAR_CELL_VOXELS[i];
            const RegularVoxelIndex voxel_index = cell_index + deltas;
            const bool inside = Density<F>::inside(regular_voxel_density(voxel_index));
            if (inside) {
                case_number += 1 << i;
            }
        }
        return case_number;
    }
    
    void extract_transition_cells() {
        
        
        for (auto side = 0; side < transition_sides.size(); ++side) {
            if(!transition_sides.test(side)) {
                continue;
            }
            current_rotation = Rotation::for_side(static_cast<TransitionSide>(side));
            for (size_t cell_u = 0; cell_u < block.subdivisions; ++cell_u) {
                for (size_t cell_v = 0; cell_v < block.subdivisions; ++cell_v) {
                    TransitionCellIndex cell_index = from_transition_side(static_cast<TransitionSide>(side), cell_u, cell_v);
                    extract_transition_cell(cell_index);
                }
            }
        }
    }

    void extract_transition_cell(TransitionCellIndex& cell_index) {
        int case_number = transition_cell_case(cell_index);
        unsigned char raw_cell_class = transitionCellClass[case_number];
        unsigned char cell_class = raw_cell_class & 0x7F;
        bool invert_triangulation = (raw_cell_class & 0x80) != 0;
        bool our_invert_triangulation = !invert_triangulation; // We use LowZ as base case so everything is inverted ?
        const auto& triangulation_info = transitionCellData[cell_class];
        const auto vertices_data = transitionVertexData[case_number];
        std::array<size_t, 12> cell_vertices_indices{0};
        for (int i = 0; i < 12; ++i) {
            if (i >= triangulation_info.GetVertexCount()) {
                break;
            }
            cell_vertices_indices[i] = transition_vertex(cell_index, TransitionVertexData{.data=vertices_data[i]});
        }
        for (int t = 0; t < triangulation_info.GetTriangleCount(); ++t) {
            int v1_index_in_cell = triangulation_info.vertexIndex[3 * t];
            int v2_index_in_cell = triangulation_info.vertexIndex[3 * t + 1];
            int v3_index_in_cell = triangulation_info.vertexIndex[3 * t + 2];
            size_t global_index_1 = cell_vertices_indices[v1_index_in_cell];
            size_t global_index_2 = cell_vertices_indices[v2_index_in_cell];
            size_t global_index_3 = cell_vertices_indices[v3_index_in_cell];
            if (our_invert_triangulation) {
                tri_indices.push_back(global_index_1);
                tri_indices.push_back(global_index_2);
                tri_indices.push_back(global_index_3);
            } else {
                tri_indices.push_back(global_index_3);
                tri_indices.push_back(global_index_2);
                tri_indices.push_back(global_index_1);
            }
        }
    }

    size_t transition_cell_case(const TransitionCellIndex& cell_index) {
        size_t case_number = 0;
        for (const auto& [voxel_delta, contribution] : TRANSITION_HIGH_RES_FACE_CASE_CONTRIBUTIONS) {
            const HighResolutionVoxelIndex voxel_index = cell_index + voxel_delta;
            const auto density = transition_grid_point_density(voxel_index);
            const bool inside = Density<F>::inside(density);
            if (inside) {
                case_number += contribution;
            }
        }
        return case_number;
    }

    D regular_voxel_density(const RegularVoxelIndex& voxel_index) {
        return density_source(voxel_index.x, voxel_index.y, voxel_index.z);
    }

    D get_transition_density(const HighResolutionVoxelIndex& index) const {
        auto rotation = Rotation::for_side(index.cell.side);
        auto position_in_block = rotation.to_position_in_block<F>(block.subdivisions, index);
        auto x = block.dims.base[0] + block.dims.size * position_in_block.x;
        auto y = block.dims.base[1] + block.dims.size * position_in_block.y;
        auto z = block.dims.base[2] + block.dims.size * position_in_block.z;
        return density_source(x, y, z);
    }

    D transition_grid_point_density(const HighResolutionVoxelIndex& voxel_index) {
        if (on_regular_grid(voxel_index)) {
            const RegularVoxelIndex regular_index = as_regular_index(voxel_index, current_rotation, block.subdivisions);
            return density_source(regular_index.x, regular_index.y, regular_index.z);
        } else {
            return get_transition_density(voxel_index);
        }
    }

    size_t regular_vertex(const RegularCellIndex& cell_index, const RegularVertexData& vd) {
        const size_t cell_x = cell_index.x;
        const size_t cell_y = cell_index.y;
        const size_t cell_z = cell_index.z;
        if (vd.new_vertex()) {
            const size_t i = new_regular_vertex(cell_index, vd.voxel_a_index(), vd.voxel_b_index());
            shared_storage.put_regular(i, cell_x, cell_y, cell_z, vd.reuse_index().index);
            return i;
        } else {
            const bool previous_vertex_is_accessible =
                ((vd.reuse_dx() == 0) || (cell_x > 0)) &&
                ((vd.reuse_dy() == 0) || (cell_y > 0)) &&
                ((vd.reuse_dz() == 0) || (cell_z > 0));
            if (previous_vertex_is_accessible) {
                return shared_storage.get_regular(
                    static_cast<size_t>(static_cast<size_t>(cell_x) + vd.reuse_dx()),
                    static_cast<size_t>(static_cast<size_t>(cell_y) + vd.reuse_dy()),
                    static_cast<size_t>(static_cast<size_t>(cell_z) + vd.reuse_dz()),
                    vd.reuse_index().index
                );
            } else {
                const size_t i = new_regular_vertex(cell_index, vd.voxel_a_index(), vd.voxel_b_index());
                return i;
            }
        }
    }

    size_t transition_vertex(const TransitionCellIndex& cell_index, const TransitionVertexData& vd) {
        if (vd.reuse()) {
            const size_t cell_u = cell_index.cell_u;
            const size_t cell_v = cell_index.cell_v;
            const bool previous_vertex_is_accessible =
                ((vd.reuse_du() == 0) || (cell_u > 0)) &&
                ((vd.reuse_dv() == 0) || (cell_v > 0));
            if (previous_vertex_is_accessible) {
                const size_t reuse_cell_u = static_cast<size_t>(static_cast<size_t>(cell_u) + vd.reuse_du());
                const size_t reuse_cell_v = static_cast<size_t>(static_cast<size_t>(cell_v) + vd.reuse_dv());
                const TransitionCellIndex previous_index = {
                    .side = cell_index.side,
                    .cell_u = reuse_cell_u,
                    .cell_v = reuse_cell_v
                };
                return shared_storage.get_transition(previous_index, vd.reuse_index());
            } else {
                const size_t i = new_transition_vertex(cell_index, vd.grid_point_a_index(), vd.grid_point_b_index());
                return i;
            }
        } else {
            const size_t i = new_transition_vertex(cell_index, vd.grid_point_a_index(), vd.grid_point_b_index());
            if (vd.new_reusable()) {
                shared_storage.put_transition(i, cell_index, vd.reuse_index());
            }
            return i;
        }
    }

    size_t new_transition_vertex(
        const TransitionCellIndex& cell_index,
        const TransitionCellGridPointIndex& grid_point_a_index,
        const TransitionCellGridPointIndex& grid_point_b_index
    ) {
        const auto a = transition_grid_point(cell_index, grid_point_a_index);
        const auto b = transition_grid_point(cell_index, grid_point_b_index);
        const size_t i = add_vertex_between(a, b);
        return i;
    }

    size_t new_regular_vertex(
        const RegularCellIndex& cell_index,
        const RegularCellVoxelIndex& voxel_a_index,
        const RegularCellVoxelIndex& voxel_b_index
    ) {
        const auto a = regular_grid_point_within_cell(cell_index, voxel_a_index);
        const auto b = regular_grid_point_within_cell(cell_index, voxel_b_index);
        return add_vertex_between(a, b);
    }

    GridPoint<F, D> regular_grid_point_within_cell(
        const RegularCellIndex& cell_index,
        const RegularCellVoxelIndex& voxel_index_within_cell
    ) {
        const auto voxel_deltas = get_regular_voxel_delta(voxel_index_within_cell);
        const RegularVoxelIndex voxel_index = cell_index + voxel_deltas;
        return regular_grid_point(voxel_index);
    }

    GridPoint<F, D> regular_grid_point(const RegularVoxelIndex& voxel_index) {
        const auto position = regular_grid_point_position(voxel_index);
        const auto gradient = regular_voxel_gradient(voxel_index);
        const D density = regular_voxel_density(voxel_index);
        return GridPoint<F, D>{.position=position, .gradient=gradient, .density=density};
    }

    Position<F> regular_grid_point_position(
        const RegularVoxelIndex& voxel_index
    ) const {
        F x = block.dims.base[0] +
            block.dims.size * Coordinate<F>::from_ratio(voxel_index.x, block.subdivisions);
        F y = block.dims.base[1] +
            block.dims.size * Coordinate<F>::from_ratio(voxel_index.y, block.subdivisions);
        F z = block.dims.base[2] +
            block.dims.size * Coordinate<F>::from_ratio(voxel_index.z, block.subdivisions);
        shrink_if_needed(x, y, z, voxel_index);
        return Position<F>(x, y, z);
    }

    void shrink_if_needed(
        F& grid_point_x,
        F& grid_point_y,
        F& grid_point_z,
        const RegularVoxelIndex& voxel_index
    ) const {
        auto cell_size = this->block.dims.size * Coordinate<F>::from_ratio(1, this->block.subdivisions);
        _shrink_if_needed<F, D>(
            grid_point_x,
            grid_point_y,
            grid_point_z,
            voxel_index.x,
            voxel_index.y,
            voxel_index.z,
            cell_size,
            this->block.subdivisions,
            this->transition_sides
        );
    }

    std::tuple<F, F, F> regular_voxel_gradient(RegularVoxelIndex voxel_index) {
        F xgradient =
            Density<F>::diff(regular_voxel_density(RegularVoxelIndex{ voxel_index.x + 1, voxel_index.y, voxel_index.z }),
            regular_voxel_density(RegularVoxelIndex{ voxel_index.x - 1, voxel_index.y, voxel_index.z }));
        F ygradient = Density<F>::diff(
            regular_voxel_density(RegularVoxelIndex{ voxel_index.x, voxel_index.y + 1, voxel_index.z }),
            regular_voxel_density(RegularVoxelIndex{ voxel_index.x, voxel_index.y - 1, voxel_index.z }));
        F zgradient = Density<F>::diff(
            regular_voxel_density(RegularVoxelIndex{ voxel_index.x, voxel_index.y, voxel_index.z + 1 })
                ,regular_voxel_density(RegularVoxelIndex{ voxel_index.x, voxel_index.y, voxel_index.z - 1 }));
        return std::make_tuple(xgradient, ygradient, zgradient);
    }

    GridPoint<F, D> transition_grid_point(const TransitionCellIndex& cell_index, const TransitionCellGridPointIndex& grid_point_index) {
        auto &tmp = TRANSITION_CELL_GRID_POINTS[grid_point_index.index];
        if(tmp.type == TransitionCellGridPointType::HighResFace) {
            return transition_grid_point_on_high_res_face(cell_index, tmp.data.highResFace);
        }
        else {
            return transition_grid_point_on_low_res_face(cell_index, tmp.data.regularFace.u, tmp.data.regularFace.v);
        }
    }

    GridPoint<F, D> transition_grid_point_on_low_res_face(
            const TransitionCellIndex& cell_index,
            const size_t face_u,
            const size_t face_v) {
        const auto rot = current_rotation;
        const auto voxel_index = rot.to_regular_voxel_index(block.subdivisions, cell_index, face_u, face_v);
        return regular_grid_point(voxel_index);
    }

    GridPoint<F, D> transition_grid_point_on_high_res_face(
        const TransitionCellIndex& cell_index, const HighResolutionVoxelDelta& delta) {
        auto voxel_index = cell_index + delta;
        auto position = high_res_face_grid_point_position(cell_index, delta);
        auto gradient = high_res_face_grid_point_gradient(voxel_index);
        auto density = high_res_face_grid_point_density(voxel_index);
        return GridPoint<F, D>{position, gradient, density};
    }

    Position<F> high_res_face_grid_point_position(const TransitionCellIndex& cell_index,
                                                const HighResolutionVoxelDelta& delta) const {
        const auto& rot = this->current_rotation;
        const auto voxel_index = cell_index + delta;
        const auto position_in_block = rot.template to_position_in_block<F>(this->block.subdivisions, voxel_index);
        const auto world_position = (position_in_block * this->block.dims.size) + this->block.dims.base;
        return world_position;
    }

    std::tuple<F, F, F> high_res_face_grid_point_gradient(HighResolutionVoxelIndex base_voxel_index) {
        if (on_regular_grid(base_voxel_index)) {
            auto regular_index = as_regular_index(base_voxel_index, current_rotation, block.subdivisions);
            return regular_voxel_gradient(regular_index);
        } else {
            return high_res_face_grid_point_gradient_non_regular(base_voxel_index);
        }
    }

    std::tuple<F, F, F> high_res_face_grid_point_gradient_non_regular(const HighResolutionVoxelIndex& base_voxel_index
    ) {
        auto &rot = current_rotation;
        auto x_gradient = transition_grid_point_density(base_voxel_index + rot.plus_x_as_uvw)
                        - transition_grid_point_density(base_voxel_index - rot.plus_x_as_uvw);
        auto y_gradient = transition_grid_point_density(base_voxel_index + rot.plus_y_as_uvw)
                        - transition_grid_point_density(base_voxel_index - rot.plus_y_as_uvw);
        auto z_gradient = transition_grid_point_density(base_voxel_index + rot.plus_z_as_uvw)
                        - transition_grid_point_density(base_voxel_index - rot.plus_z_as_uvw);
        return {x_gradient, y_gradient, z_gradient};
    }

    D high_res_face_grid_point_density(HighResolutionVoxelIndex voxel_index) {
        return transition_grid_point_density(voxel_index);
    }

    size_t add_vertex_between(GridPoint<F, D> point_a, GridPoint<F, D> point_b) {
        auto interp_toward_b = Density<F>::interp(point_a.density, point_b.density, threshold);
        auto position = point_a.position.interp_toward(point_b.position, interp_toward_b);
        auto gradient_x = std::get<0>(point_a.gradient) + interp_toward_b * (std::get<0>(point_b.gradient) - std::get<0>(point_a.gradient));
        auto gradient_y = std::get<1>(point_a.gradient) + interp_toward_b * (std::get<1>(point_b.gradient) - std::get<1>(point_a.gradient));
        auto gradient_z = std::get<2>(point_a.gradient) + interp_toward_b * (std::get<2>(point_b.gradient) - std::get<2>(point_a.gradient));
        auto normal = Density<F>::to_normal(gradient_x, gradient_y, gradient_z);
        vertices_positions.push_back(position.x);
        vertices_positions.push_back(position.y);
        vertices_positions.push_back(position.z);
        vertices_normals.push_back(std::get<0>(normal));
        vertices_normals.push_back(std::get<1>(normal));
        vertices_normals.push_back(std::get<2>(normal));
        auto index = vertices;
        vertices += 1;
        return index;
    }
};