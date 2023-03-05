#include <cmath>
#include <iostream>
#include "transvoxel/density.hpp"
#include "transvoxel/extraction.hpp"
#include "transvoxel/structs.hpp"
#include "transvoxel/voxel_source.hpp"

float sphere_density(float x, float y, float z) {
    return 1.0f - std::sqrt(x * x + y * y + z * z) / 5.0f;
}

struct Sphere : public ScalarField<float, float> {
    float get_density(float x, float y, float z) const override {
        return sphere_density(x, y, z);
    }
};

const float THRESHOLD = 0.0f;

int main() {

    // Extraction parameters: world zone and subdivisions
    const int subdivisions = 3;
    const Block<float> block({0.0f, 0.0f, 0.0f}, 10.0f, subdivisions);

    // Extract from a VoxelSource
    WorldMappingVoxelSource<float, float, Sphere> source(Sphere{}, block);
    auto mesh = extract(source, block, THRESHOLD, into(TransitionSide::LowX));
    std::cout << "Extracted mesh: " << mesh << std::endl;

    // // Extract from a ScalarField
    // Sphere field;
    // mesh = extract(field, block, THRESHOLD, into(TransitionSide::LowX));
    // std::cout << "Extracted mesh: " << mesh << std::endl;

    // // Extract from a simple field function
    // transvoxel::ScalarFieldForFn<float, float> field(sphere_density);
    // mesh = transvoxel::extract(field, block, THRESHOLD, transvoxel::TransitionSide::LowX);
    // std::cout << "Extracted mesh: " << mesh << std::endl;

    // // Extract from a simple field closure
    // transvoxel::ScalarFieldForFn<float, float> closure([](float x, float y, float z) {
    //     return 1.0f - std::sqrt(x * x + y * y + z * z) / 5.0f;
    // });
    // mesh = transvoxel::extract(closure, block, THRESHOLD, transvoxel::TransitionSide::LowX);
    // std::cout << "Extracted mesh: " << mesh << std::endl;

    return 0;
}