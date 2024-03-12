#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Vertex3D {
  glm::vec3 position;
  glm::vec2 texCoords;
  glm::vec3 normal;
  glm::vec4 color;

  Vertex3D(glm::vec3 position, glm::vec2 texCoords, glm::vec3 normal, glm::vec4 color)
      : position(position), texCoords(texCoords), normal(normal), color(color) {}

  Vertex3D(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a) {
    position = glm::vec3(x, y, z);
    texCoords = glm::vec2(u, v);
    normal = glm::vec3(nx, ny, nz);
    color = glm::vec4(r, g, b, a);
  }

  Vertex3D() {}
};

struct Material {
  glm::vec3 baseColorFactor;
  float metallicFactor;
  float roughnessFactor;
  glm::vec3 emissiveFactor;
  float emissiveStrength;
};

struct Mesh {
  Mesh(std::vector<Vertex3D> vertices, std::vector<GLuint> indices);
  ~Mesh();

  GLuint vbo;
  GLuint ebo;
  GLuint vao;

  std::vector<Vertex3D> vertices;
  std::vector<GLuint> indices;

  glm::mat4 model = glm::mat4(1.0f);
  std::shared_ptr<Material> material;
};
