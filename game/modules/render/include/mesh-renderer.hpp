#pragma once
#include "shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "mesh.hpp"

class MeshRenderer {
public:
  MeshRenderer();

  void DrawMesh(Mesh *mesh, glm::mat4 model);

  void SetViewMatrix(glm::mat4 viewMatrix);

private:
  void setMaterialUniforms(GLuint shaderProgram, const Material *material);
  Shader vertexShader;
  Shader fragmentShader;
  GLuint shaderProgram;
};