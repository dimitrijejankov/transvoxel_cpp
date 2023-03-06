#include <cmath>
#include <iostream>
#include "transvoxel/density.hpp"
#include "transvoxel/extraction.hpp"
#include "transvoxel/structs.hpp"

float sphere_density(float x, float y, float z) {
    return 1.0f - std::sqrt(x * x + y * y + z * z) / 5.0f;
}

const float THRESHOLD = 0.0f;

int main() {

    // Extraction parameters: world zone and subdivisions
    const int subdivisions = 3;
    const Block<float> block({0.0f, 0.0f, 0.0f}, 10.0f, subdivisions);

    // Extract from a VoxelSource
    auto mesh = extract(sphere_density, block, THRESHOLD, into(TransitionSide::LowX));
    std::cout << "Extracted mesh: " << mesh << std::endl;

    return 0;
}