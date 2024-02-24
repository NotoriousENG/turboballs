#pragma once
#include "shader.hpp"
#include "texture.hpp"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex3D {
  glm::vec3 position;
  glm::vec2 texCoords;
  glm::vec3 normal;

  Vertex3D(glm::vec3 position, glm::vec2 texCoords)
      : position(position), texCoords(texCoords), normal(normal) {}

  Vertex3D(float x, float y, float z, float u, float v, float nx, float ny,
           float nz) {
    position = glm::vec3(x, y, z);
    texCoords = glm::vec2(u, v);
    normal = glm::vec3(nx, ny, nz);
  }

  Vertex3D() {}
};

class MeshRenderer {
public:
  MeshRenderer();
  ~MeshRenderer();

  void Draw();

private:
  void loadModel();

  std::vector<Vertex3D> vertices;
  GLuint vbo;

  std::vector<GLuint> indices;
  GLuint ebo;

  GLuint vao;

  Shader vertexShader;
  Shader fragmentShader;

  GLuint shaderProgram;

  glm::mat4 model;
};