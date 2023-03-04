#pragma once

#include <array>
#include <ostream>
#include <vector>

template <typename F> struct BlockDims {
  static constexpr int kDimension = 3;

  BlockDims(const std::array<F, kDimension> &base, F size)
      : base(base), size(size) {}

  std::array<F, kDimension> base;
  F size;
};

template <typename F> struct Block {
  Block(const BlockDims<F> &dims, size_t subdivisions)
      : dims(dims), subdivisions(subdivisions) {}

  Block(const std::array<F, 3> &base, F size, size_t subdivisions)
      : dims(BlockDims<F>(base, size)), subdivisions(subdivisions) {}

  BlockDims<F> dims;
  size_t subdivisions;
};

template <typename F> struct Vertex {
  std::array<F, 3> position;
  std::array<F, 3> normal;
};

template <typename F> struct Triangle {
  std::array<Vertex<F>, 3> vertices;
};

template <typename F> struct Mesh {
  std::vector<F> positions;
  std::vector<F> normals;
  std::vector<size_t> triangle_indices;

  Mesh() = default;
  Mesh(const std::vector<F> &positions, const std::vector<F> &normals,
       const std::vector<size_t> &triangle_indices)
      : positions(positions), normals(normals),
        triangle_indices(triangle_indices) {}

  size_t num_tris() const { return triangle_indices.size() / 3; }

  std::vector<Triangle<F>> tris() const {
    std::vector<Triangle<F>> tris;
    for (size_t i = 0; i < num_tris(); ++i) {
      size_t i1 = triangle_indices[3 * i];
      size_t i2 = triangle_indices[3 * i + 1];
      size_t i3 = triangle_indices[3 * i + 2];
      tris.push_back(Triangle<F>{{
          Vertex<F>{
              {positions[3 * i1], positions[3 * i1 + 1], positions[3 * i1 + 2]},
              {normals[3 * i1], normals[3 * i1 + 1], normals[3 * i1 + 2]}},
          Vertex<F>{
              {positions[3 * i2], positions[3 * i2 + 1], positions[3 * i2 + 2]},
              {normals[3 * i2], normals[3 * i2 + 1], normals[3 * i2 + 2]}},
          Vertex<F>{
              {positions[3 * i3], positions[3 * i3 + 1], positions[3 * i3 + 2]},
              {normals[3 * i3], normals[3 * i3 + 1], normals[3 * i3 + 2]}},
      }});
    }
    return tris;
  }
};

template <typename F>
std::ostream &operator<<(std::ostream &os, const Triangle<F> &triangle) {
  os << "Triangle:" << std::endl;
  for (const auto &vertex : triangle.vertices) {
    os << "    + Pos " << vertex.position[0] << " " << vertex.position[1] << " "
       << vertex.position[2] << "  Norm " << vertex.normal[0] << " "
       << vertex.normal[1] << " " << vertex.normal[2] << std::endl;
  }
  return os;
}

template <typename F>
std::ostream &operator<<(std::ostream &os, const Mesh<F> &mesh) {

  auto triangles = mesh.tris();
  for(auto &tri : triangles) {
    os << tri;
  }
  return os;
}

template <typename F> struct Position {
  F x, y, z;

  Position(F x, F y, F z) : x(x), y(y), z(z) {}

  Position interp_toward(const Position &other, F factor) const {
    return Position(x + factor * (other.x - x), y + factor * (other.y - y),
                    z + factor * (other.z - z));
  }
};

template <typename F> Position<F> operator*(const Position<F> &pos, F factor) {
  return Position<F>(pos.x * factor, pos.y * factor, pos.z * factor);
}

template <typename F>
Position<F> operator+(const Position<F> &pos, const std::array<F, 3> &vec) {
  return Position<F>(pos.x + vec[0], pos.y + vec[1], pos.z + vec[2]);
}