#include <cassert>
#include <cstddef>
#include <vector>
#include <unordered_map>
#include "../voxel_coordinates.hpp"

template<typename D, typename S>
struct PreCachingVoxelSource {

    S inner_source;
    size_t block_subdivisions;
    std::vector<D> regular_cache;
    std::vector<D> regular_cache_extended;
    bool regular_cache_extended_loaded;
    std::vector<D> transition_cache;
    bool transition_cache_loaded;
    std::unordered_map<size_t, size_t> transition_cache_slices; // side -> slice in the cache

    PreCachingVoxelSource(S source, size_t block_subdivisions)
    : inner_source(std::move(source)),
      block_subdivisions(block_subdivisions),
      regular_cache(),
      regular_cache_extended(),
      regular_cache_extended_loaded(false),
      transition_cache(),
      transition_cache_loaded(false),
      transition_cache_slices() 
    {
        load_regular_block_voxels();
    }

    void load_regular_block_voxels() {
        const size_t subs = block_subdivisions;
        regular_cache.resize((subs + 1) * (subs + 1) * (subs + 1));
        for (size_t x = 0; x <= subs; ++x) {
            for (size_t y = 0; y <= subs; ++y) {
                for (size_t z = 0; z <= subs; ++z) {
                    const size_t index = regular_block_index(x, y, z);
                    regular_cache[index] = from_source(x, y, z);
                }
            }
        }
    }

    size_t regular_block_index(size_t x, size_t y, size_t z) const {
        const size_t subs = block_subdivisions;
        return (subs + 1) * (subs + 1) * x + (subs + 1) * y + z;
    }

    D from_source(size_t x, size_t y, size_t z) {
        return inner_source.get_density(RegularVoxelIndex{ x, y, z });
    }

    void load_regular_extended_voxels() {
        if (regular_cache_extended_loaded) {
            return;
        } else {
            regular_cache_extended_loaded = true;
        }
        const size_t subs = block_subdivisions;
        const size_t face_size = (subs + 1) * (subs + 1);
        regular_cache_extended.resize(6 * face_size);
        // -x
        for (size_t y = 0; y <= subs; ++y) {
            for (size_t z = 0; z <= subs; ++z) {
                const size_t index = 0 * face_size + (subs + 1) * y + z;
                regular_cache_extended[index] = from_source(-1, y, z);
            }
        }
        // +x
        for (size_t y = 0; y <= subs; ++y) {
            for (size_t z = 0; z <= subs; ++z) {
                const size_t index = 1 * face_size + (subs + 1) * y + z;
                regular_cache_extended[index] = from_source(subs + 1, y, z);
            }
        }
        // -y
        for (size_t x = 0; x <= subs; ++x) {
            for (size_t z = 0; z <= subs; ++z) {
                const size_t index = 2 * face_size + (subs + 1) * x + z;
                regular_cache_extended[index] = from_source(x, -1, z);
            }
        }
        // +y
        for (size_t x = 0; x <= subs; ++x) {
            for (size_t z = 0; z <= subs; ++z) {
                const size_t index = 3 * face_size + (subs + 1) * x + z;
                regular_cache_extended[index] = from_source(x, subs + 1, z);
            }
        }
        // -z
        for (size_t x = 0; x <= subs; ++x) {
            for (size_t y = 0; y <= subs; ++y) {
                const size_t index = 4 * face_size + (subs + 1) * x + y;
                regular_cache_extended[index] = from_source(x, y, -1);
            }
        }
        // +z
        for (size_t x = 0; x <= subs; ++x) {
            for (size_t y = 0; y <= subs; ++y) {
                const size_t index = 5 * face_size + (subs + 1) * x + y;
                regular_cache_extended[index] = from_source(x, y, subs + 1);
            }
        }
    }

    void load_transition_voxels(TransitionSides transition_sides) {
        if (transition_cache_loaded) {
            return;
        } else {
            transition_cache_loaded = true;
        }
        size_t num_transitions = 0;
        // for (auto side : transition_sides) {
        for(uint8_t side = 0; side < 6; ++side) {
            if(!transition_sides.test(side)) {
                continue;
            }
            transition_cache_slices.insert({ side, num_transitions });
            ++num_transitions;
        }
        // We will only store the w=0 voxels, and not the ones out of the block (so, all voxels for case computations, and vertex positions, but not for gradients)
        // For simplicity (at the cost of compactness) we store a sparse array also containing regular voxels on the face, that will never get read/written
        const size_t subs = block_subdivisions;
        const size_t size_per_face = (2 * subs + 1) * (2 * subs + 1);
        transition_cache.resize(num_transitions * size_per_face);
        for(uint8_t side = 0; side < 6; ++side) {
            if(!transition_sides.test(side)) {
                continue;
            }
            for (size_t cell_u = 0; cell_u < subs; ++cell_u) {
                for (size_t cell_v = 0; cell_v < subs; ++cell_v) {
                    cache_transition_voxel(from_transition_side(
                        static_cast<TransitionSide>(side), cell_u, cell_v, 1, 0, 0));
                    cache_transition_voxel(from_transition_side(
                        static_cast<TransitionSide>(side), cell_u, cell_v, 0, 1, 0));
                    cache_transition_voxel(from_transition_side(
                        static_cast<TransitionSide>(side), cell_u, cell_v, 1, 1, 0));
                }
                cache_transition_voxel(from_transition_side(
                    static_cast<TransitionSide>(side), cell_u, subs - 1, 1, 2, 0));
            }
            for (size_t cell_v = 0; cell_v < subs; ++cell_v) {
                cache_transition_voxel(from_transition_side(
                    static_cast<TransitionSide>(side), subs - 1, cell_v, 2, 1, 0));
            }
        }
    }

    void cache_transition_voxel(const HighResolutionVoxelIndex& voxel_index) {
        const D d = inner_source.get_transition_density(voxel_index);
        const size_t cache_index = transition_cache_index(voxel_index);
        transition_cache[cache_index] = d;
    }

    size_t transition_cache_index(const HighResolutionVoxelIndex& voxel_index) const {
        const size_t side = static_cast<size_t>(voxel_index.cell.side);
        const size_t cache_slice = transition_cache_slices.at(side);
        const size_t subs = block_subdivisions;
        const size_t size_per_face = (2 * subs + 1) * (2 * subs + 1);
        const size_t slice_shift = cache_slice * size_per_face;
        const size_t global_du = 2 * voxel_index.cell.cell_u + voxel_index.delta.u;
        const size_t global_dv = 2 * voxel_index.cell.cell_v + voxel_index.delta.v;
        const size_t index_in_slice = (2 * subs + 1) * static_cast<size_t>(global_du) +
        static_cast<size_t>(global_dv);
        return slice_shift + index_in_slice;
    }

    D get_density(const RegularVoxelIndex& voxel_index) const {
        const size_t x = voxel_index.x;
        const size_t y = voxel_index.y;
        const size_t z = voxel_index.z;
        const size_t subs = block_subdivisions;
        const size_t face_size = (subs + 1) * (subs + 1);

        if (x == -1) {
            assert(y >= 0 && y <= static_cast<size_t>(subs) && z >= 0 && z <= static_cast<size_t>(subs));
            const size_t index = 0 * face_size + (subs + 1) * static_cast<size_t>(y) + static_cast<size_t>(z);
            return regular_cache_extended[index];
        } else if (x == static_cast<size_t>(subs) + 1) {
            assert(y >= 0 && y <= static_cast<size_t>(subs) && z >= 0 && z <= static_cast<size_t>(subs));
            const size_t index = 1 * face_size + (subs + 1) * static_cast<size_t>(y) + static_cast<size_t>(z);
            return regular_cache_extended[index];
        } else if (y == -1) {
            assert(x >= 0 && x <= static_cast<size_t>(subs) && z >= 0 && z <= static_cast<size_t>(subs));
            const size_t index = 2 * face_size + (subs + 1) * static_cast<size_t>(x) + static_cast<size_t>(z);
            return regular_cache_extended[index];
        } else if (y == static_cast<size_t>(subs) + 1) {
            assert(x >= 0 && x <= static_cast<size_t>(subs) && z >= 0 && z <= static_cast<size_t>(subs));
            const size_t index = 3 * face_size + (subs + 1) * static_cast<size_t>(x) + static_cast<size_t>(z);
            return regular_cache_extended[index];
        } else if (z == -1) {
            assert(x >= 0 && x <= static_cast<size_t>(subs) && y >= 0 && y <= static_cast<size_t>(subs));
            const size_t index = 4 * face_size + (subs + 1) * static_cast<size_t>(x) + static_cast<size_t>(y);
            return regular_cache_extended[index];
        } else if (z == static_cast<size_t>(subs) + 1) {
            assert(x >= 0 && x <= static_cast<size_t>(subs) && y >= 0 && y <= static_cast<size_t>(subs));
            const size_t index = 5 * face_size + (subs + 1) * static_cast<size_t>(x) + static_cast<size_t>(y);
            return regular_cache_extended[index];
        } else {
            assert(x >= 0 && x <= static_cast<size_t>(subs) && y >= 0 && y <= static_cast<size_t>(subs) &&
            z >= 0 && z <= static_cast<size_t>(subs));
            const size_t index = regular_block_index(x, y, z);
            return regular_cache[index];
        }
    }

    D get_transition_density(const HighResolutionVoxelIndex& index) const {
        const auto c = index.cell;
        const auto d = index.delta;
        const auto subs = static_cast<size_t>(block_subdivisions);
        assert(d.w != 0 || d.u % 2 != 0 || d.v % 2 != 0);
        // The following check is only valid if, for voxels coinciding with a regular voxel,
        // we also get the gradient from the regular voxel. Not sure this is correct (see
        // `high_res_face_grid_point_gradient`)
        if (d.w != 0) {
            assert(d.u % 2 != 0 || d.v % 2 != 0);
        }
        if (d.w != 0 || c.cell_u * 2 + d.u < 0 || c.cell_u * 2 + d.u > 2 * subs ||
            c.cell_v * 2 + d.v < 0 || c.cell_v * 2 + d.v > 2 * subs) {
            // Out of the block face: we don't cache these
            return inner_source.get_transition_density(index);
        }
        const auto cache_index = transition_cache_index(index);
        return transition_cache[cache_index];
    }
};