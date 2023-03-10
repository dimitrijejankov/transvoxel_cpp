#include <cmath>
#include <cstddef>
#include <iostream>
#include <fstream>
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

Mesh<float> load() {

    Mesh<float> mesh;

    std::ifstream infile("../tests/sphere_10_3.txt");

    // load num position
    size_t num_pos;
    infile >> num_pos;
    mesh.positions.resize(num_pos);

    // load all positions
    for(size_t idx = 0; idx < num_pos; ++idx) {
        float tmp;
        infile >> tmp;
        mesh.positions[idx] = tmp;
    }

    // load num position
    size_t num_normals;
    infile >> num_normals;
    mesh.normals.resize(num_normals);

    // load all positions
    for(size_t idx = 0; idx < num_normals; ++idx) {
        float tmp;
        infile >> tmp;
        mesh.normals[idx] = tmp;
    }

    // load all triangle indices
    size_t triangle_indices_num;
    infile >> triangle_indices_num;
    mesh.triangle_indices.resize(triangle_indices_num);

    for(size_t idx = 0; idx < triangle_indices_num; ++idx) {
        size_t tmp;
        infile >> tmp;
        mesh.triangle_indices[idx] = tmp;
    }

    infile.close();

    return mesh;
}

#define FLOAT_EQ(a, b, epsilon) (std::abs((a) - (b)) < (epsilon))

int main() {

    // Extraction parameters: world zone and subdivisions
    const int subdivisions = 3;
    const Block<float> block({0.0f, 0.0f, 0.0f}, 10.0f, subdivisions);

    // Extract from a VoxelSource
    WorldMappingVoxelSource<float, float, Sphere> source(Sphere{}, block);
    auto mesh = extract(source, block, THRESHOLD, into(TransitionSide::LowX));

    auto test_mesh = load();

    for (auto const& n : mesh.positions) {
        if(!FLOAT_EQ(mesh.positions[n], test_mesh.positions[n], 0.0001f)) {
            std::cout << mesh.positions[n] << " " << test_mesh.positions[n] << std::endl;
        }
    }

    for (auto const& n : mesh.normals) {
        if(!FLOAT_EQ(mesh.normals[n], test_mesh.normals[n], 0.0001f)) {
            std::cout << mesh.normals[n] << " " << test_mesh.normals[n] << std::endl;
        }
    }

    for (auto const& n : mesh.triangle_indices) {
        if(mesh.triangle_indices[n] != test_mesh.triangle_indices[n]) {
            std::cout << mesh.triangle_indices[n] << " " << test_mesh.triangle_indices[n] << std::endl;
        }
    }
    
    return 0;
}