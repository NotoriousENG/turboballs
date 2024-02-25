#pragma once
#include "shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "mesh.hpp"

class MeshRenderer {
public:
  MeshRenderer();

  void DrawMesh(Mesh *mesh, glm::mat4 model);

private:
  Shader vertexShader;
  Shader fragmentShader;
  GLuint shaderProgram;
};