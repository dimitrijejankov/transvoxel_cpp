#pragma once

#include <bitset>
#include <iostream>

enum class TransitionSide : uint8_t {
    LowX,
    HighX,
    LowY,
    HighY,
    LowZ,
    HighZ,
};

using TransitionSides = std::bitset<6>;

TransitionSides no_side();

TransitionSides into(TransitionSide side);
