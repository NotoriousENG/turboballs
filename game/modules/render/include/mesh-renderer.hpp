#pragma once
#include "shader.hpp"
#include "texture.hpp"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex3D {
  glm::vec3 position;

  Vertex3D(glm::vec3 position) : position(position) {}

  Vertex3D(float x, float y, float z) { position = glm::vec3(x, y, z); }
};

class MeshRenderer {
public:
  MeshRenderer();
  ~MeshRenderer();

  void Draw();

private:
  std::vector<Vertex3D> vertices = {
      // front top left
      Vertex3D(-0.5, 0.5, 0.5),
      // front top left
      Vertex3D(0.5, 0.5, 0.5),
      // front bottom right
      Vertex3D(0.5, -0.5, 0.5),
      // front bottom left
      Vertex3D(-0.5, -0.5, 0.5),
      // back top left
      Vertex3D(-0.5, 0.5, -0.5),
      // back top right
      Vertex3D(0.5, 0.5, -0.5),
      // back bottom right
      Vertex3D(0.5, -0.5, -0.5),
      // back bottom left
      Vertex3D(-0.5, -0.5, -0.5),
  };
  GLuint vbo;

  std::vector<GLuint> indices = {
      0, 3, 2, // Front 1
      2, 1, 0, // Front 2
      1, 5, 6, // Right 1
      6, 2, 1, // Right 2
      5, 4, 7, // Left 1
      7, 6, 5, // Left 2
      4, 7, 3, // Back 1
      3, 0, 4, // Back 2
      4, 5, 1, // Top 1
      1, 0, 4, // Top 2
      3, 2, 6, // Bottom 1
      6, 7, 3  // Bottom 2,
  };
  GLuint ebo;

  GLuint vao;

  Shader vertexShader;
  Shader fragmentShader;

  GLuint shaderProgram;
};