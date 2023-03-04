#include "aux_tables.hpp"

std::array<Rotation, 6> ROTATIONS = {
    Rotation{
        TransitionSide::LowX,
        {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {1, 0, 0},
        {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {-1, 0, 0}}, // +X is +W, +Y is +V, +Z is -U
    Rotation{
        TransitionSide::HighX,
        {1, 0, 0}, {0, 0, 1}, {0, 1, 0}, {-1, 0, 0},
        {0, 0, 1}, {0, 0, -1}, {0, 1, 0}, {1, 0, 0}},
    Rotation{
        TransitionSide::LowY,
        {0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {0, 1, 0}, // U: +X, V: -Z, W: +Y
        {0, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, -1, 0}},
    Rotation{
        TransitionSide::HighY,
        {0, 1, 0}, {1, 0, 0}, {0, 0, 1}, {0, -1, 0},
        {0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {0, 1, 0}},
    Rotation{
        TransitionSide::LowZ,
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, // U: +X, V: +Y, W: +Z
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
    Rotation{
        TransitionSide::HighZ,
        {1, 0, 1}, {-1, 0, 0}, {0, 1, 0}, {0, 0, -1},
        {1, 0, 1}, {-1, 0, 0}, {0, 1, 0}, {0, 0, -1}},
};

std::array<TransitionCellGridPoint, 13> TRANSITION_CELL_GRID_POINTS = {
    tcell_highres_face_gridpoint(0, 0),
    tcell_highres_face_gridpoint(1, 0),
    tcell_highres_face_gridpoint(2, 0),
    tcell_highres_face_gridpoint(0, 1),
    tcell_highres_face_gridpoint(1, 1),
    tcell_highres_face_gridpoint(2, 1),
    tcell_highres_face_gridpoint(0, 2),
    tcell_highres_face_gridpoint(1, 2),
    tcell_highres_face_gridpoint(2, 2),
    tcell_reg_face_gridpoint(0, 0),
    tcell_reg_face_gridpoint(1, 0),
    tcell_reg_face_gridpoint(0, 1),
    tcell_reg_face_gridpoint(1, 1)
};

std::array<std::pair<HighResolutionVoxelDelta, size_t>, 9> TRANSITION_HIGH_RES_FACE_CASE_CONTRIBUTIONS = {
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({0, 0, 0}), 0x01 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({1, 0, 0}), 0x02 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({2, 0, 0}), 0x04 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({0, 1, 0}), 0x80 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({1, 1, 0}), 0x100 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({2, 1, 0}), 0x08 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({0, 2, 0}), 0x40 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({1, 2, 0}), 0x20 },
    std::pair<HighResolutionVoxelDelta, size_t>{ HighResolutionVoxelDelta({2, 2, 0}), 0x10 }
};
