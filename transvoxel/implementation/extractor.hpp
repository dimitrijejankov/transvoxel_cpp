#pragma once

#include "density_caching.hpp"
#include "rotation.hpp"
#include "aux_tables.hpp"
#include "table_wrappers.hpp"
#include <cstddef>
#include <cstdint>


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
void shrink_if_needed(F &x, F &y, F &z,
                      size_t xi, size_t yi, size_t zi, F cell_size,
                      size_t subdivisions, const TransitionSides &transition_sides) {
    typename D::F shrink = D::shrink_factor() * cell_size;
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
    PreCachingVoxelSource<D, S> density_source;
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
        : density_source(PreCachingVoxelSource<D, S>(density_source, block.subdivisions)),
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

    // void load_regular_block_voxels()
    // {
    //     auto subs = block_subdivisions;
    //     regular_cache.resize((subs + 1) * (subs + 1) * (subs + 1), D::default());
    //     for (int x = 0; x <= subs; x++)
    //     {
    //         for (int y = 0; y <= subs; y++)
    //         {
    //             for (int z = 0; z <= subs; z++)
    //             {
    //                 auto index = regular_block_index(x, y, z);
    //                 regular_cache[index] = from_source(x, y, z);
    //             }
    //         }
    //     }
    // }

};