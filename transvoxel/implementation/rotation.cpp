#include "rotation.hpp"
#include "aux_tables.hpp"

const Rotation& Rotation::for_side(TransitionSide side) {
    return ROTATIONS[static_cast<std::size_t>(side)];
}

const Rotation& Rotation::create_default() {
    return ROTATIONS[0];
}