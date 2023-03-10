#pragma once
#include "rotation.hpp"
#include <array>
#include <cstdint>

extern std::array<Rotation, 6> ROTATIONS;

struct RegularCellVoxelIndex {
    int index;
    RegularCellVoxelIndex(int i) : index(i) {}
};

const std::array<RegularVoxelDelta, 8> REGULAR_CELL_VOXELS = {
    RegularVoxelDelta{ 0, 0, 0 }, // Voxel 0 is the cell "origin" [with the lowest x, y, and z]
    RegularVoxelDelta{ 1, 0, 0 }, // Voxel 1 == 1 toward X
    RegularVoxelDelta{ 0, 1, 0 }, // Voxel 2 == 1 toward Y
    RegularVoxelDelta{ 1, 1, 0 },
    RegularVoxelDelta{ 0, 0, 1 },
    RegularVoxelDelta{ 1, 0, 1 },
    RegularVoxelDelta{ 0, 1, 1 },
    RegularVoxelDelta{ 1, 1, 1 }
};

inline RegularVoxelDelta get_regular_voxel_delta(const RegularCellVoxelIndex& index) {
    return REGULAR_CELL_VOXELS[index.index];
}

struct TransitionCellGridPointIndex {
    size_t index;
};

enum class TransitionCellGridPointType {
    HighResFace,
    RegularFace
};

struct TransitionCellGridPoint {
    TransitionCellGridPointType type;
    union {
        HighResolutionVoxelDelta highResFace;
        struct {
            size_t u, v;
        } regularFace;
    } data;

    TransitionCellGridPoint(TransitionCellGridPointType type, HighResolutionVoxelDelta highResFace) : type(type) {
        data.highResFace = highResFace;
    }

    TransitionCellGridPoint(TransitionCellGridPointType type, size_t u, size_t v) : type(type) {
        data.regularFace.u = u;
        data.regularFace.v = v;
    }
};

inline TransitionCellGridPoint tcell_highres_face_gridpoint(int64_t u, int64_t v) {
    return TransitionCellGridPoint(TransitionCellGridPointType::HighResFace, HighResolutionVoxelDelta({u, v, 0}));
}

inline TransitionCellGridPoint tcell_reg_face_gridpoint(size_t u, size_t v) {
    return TransitionCellGridPoint(TransitionCellGridPointType::RegularFace, u, v);
}

extern std::array<TransitionCellGridPoint, 13> TRANSITION_CELL_GRID_POINTS;

extern std::array<std::pair<HighResolutionVoxelDelta, size_t>, 9> TRANSITION_HIGH_RES_FACE_CASE_CONTRIBUTIONS;