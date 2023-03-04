#include "transition_sides.hpp"

TransitionSides no_side() {
    return TransitionSides();
}

TransitionSides into(TransitionSide side) {
    return TransitionSides{1lu << static_cast<int>(side)};
}