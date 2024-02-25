#include "mesh.hpp"

#include <SDL.h>

Mesh::Mesh(std::vector<Vertex3D> vertices, std::vector<GLuint> indices) {
  this->vertices = vertices;
  this->indices = indices;

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
}
