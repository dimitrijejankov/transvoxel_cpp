#pragma once

#include <cstddef>
#include "aux_tables.hpp"

struct RegularReuseIndex {
    size_t index;
};

struct TransitionReuseIndex {
    size_t index;
};

struct RegularVertexData {
    uint16_t data;

    int64_t reuse_dx() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return -(((reuse_info & 0x10) >> 4) ? 1 : 0);
    }

    int64_t reuse_dy() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return -(((reuse_info & 0x20) >> 5) ? 1 : 0);
    }

    int64_t reuse_dz() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return -(((reuse_info & 0x40) >> 6) ? 1 : 0);
    }

    bool new_vertex() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return (reuse_info & 0x80) != 0;
    }

    RegularReuseIndex reuse_index() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return RegularReuseIndex{ static_cast<size_t>(reuse_info & 0x0F) };
    }

    RegularCellVoxelIndex voxel_a_index() const {
        uint8_t edge_location = data & 0xFF;
        return RegularCellVoxelIndex( static_cast<size_t>((edge_location & 0xF0) >> 4) );
    }

    RegularCellVoxelIndex voxel_b_index() const {
        uint8_t edge_location = data & 0xFF;
        return RegularCellVoxelIndex( static_cast<size_t>(edge_location & 0xF));
    }
};

struct TransitionVertexData {
    uint16_t data;

    bool reuse() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return (reuse_info & 0x30) != 0;
    }

    int64_t reuse_du() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return -(((reuse_info & 0x10) >> 4) ? 1 : 0);
    }

    int64_t reuse_dv() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return -(((reuse_info & 0x20) >> 5) ? 1 : 0);
    }

    bool new_interior() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return (reuse_info & 0x40) != 0;
    }

    bool new_reusable() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return (reuse_info & 0x80) != 0;
    }

    TransitionReuseIndex reuse_index() const {
        uint8_t reuse_info = (data & 0xFF00) >> 8;
        return TransitionReuseIndex{ static_cast<size_t>(reuse_info & 0x0F) };
    }

    TransitionCellGridPointIndex grid_point_a_index() const {
        uint8_t edge_location = data & 0xFF;
        return TransitionCellGridPointIndex{ static_cast<size_t>((edge_location & 0xF0) >> 4) };
    }

    TransitionCellGridPointIndex grid_point_b_index() const {
        uint8_t edge_location = data & 0xFF;
        return TransitionCellGridPointIndex{ static_cast<size_t>(edge_location & 0xF) };
    }
};