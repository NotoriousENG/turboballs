#include "mesh-renderer.hpp"

#include <SDL.h>
#include <glm/gtc/type_ptr.hpp>

MeshRenderer::MeshRenderer() {

  this->vertexShader.LoadFromFile("assets/shaders/mesh.vert", GL_VERTEX_SHADER);
  this->fragmentShader.LoadFromFile("assets/shaders/mesh.frag",
                                    GL_FRAGMENT_SHADER);

  this->shaderProgram = glCreateProgram();

  this->vertexShader.AttatchToProgram(this->shaderProgram);
  this->fragmentShader.AttatchToProgram(this->shaderProgram);

  glLinkProgram(this->shaderProgram);

  GLint linkStatus;

  glGetProgramiv(this->shaderProgram, GL_LINK_STATUS, &linkStatus);

  if (linkStatus != GL_TRUE) {
    GLint logLength;
    glGetProgramiv(this->shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<GLchar> logBuffer(logLength);
    glGetProgramInfoLog(this->shaderProgram, logLength, nullptr,
                        logBuffer.data());
    std::string log(logBuffer.begin(), logBuffer.end());
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to link shader program: %s", log.c_str());
    glDeleteProgram(this->shaderProgram);
    return;
  }

  glUseProgram(this->shaderProgram);
  // set mat4 view
  GLint viewLoc = glGetUniformLocation(this->shaderProgram, "view");
  glm::mat4 view =
      glm::lookAt(glm::vec3(0.0f, 2.85f, 15.63f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  // set mat4 projection
  GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
  glm::mat4 projection =
      glm::perspective(glm::radians(50.0f), 800.0f / 600.0f, 0.1f, 100.0f);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void MeshRenderer::DrawMesh(Mesh *mesh, glm::mat4 model) {
  glUseProgram(this->shaderProgram);
  GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  if (mesh->material) {
    setMaterialUniforms(this->shaderProgram, mesh->material.get());
  }

  glBindVertexArray(mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex3D),
               mesh->vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(GLuint),
               mesh->indices.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)offsetof(Vertex3D, position));
  glEnableVertexAttribArray(0);

  // texture coord attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)offsetof(Vertex3D, texCoords));
  glEnableVertexAttribArray(1);

  // normal attribute
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)offsetof(Vertex3D, normal));
  glEnableVertexAttribArray(2);

  // color attribute
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)offsetof(Vertex3D, color));
  glEnableVertexAttribArray(3);

  glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void MeshRenderer::SetViewMatrix(glm::mat4 viewMatrix) {
  glUseProgram(this->shaderProgram);
  GLint viewLoc = glGetUniformLocation(this->shaderProgram, "view");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
  glUseProgram(0);
}

void MeshRenderer::setMaterialUniforms(GLuint shaderProgram,
                                       const Material *material) {
  glUniform3fv(glGetUniformLocation(shaderProgram, "material.baseColorFactor"),
               1, glm::value_ptr(material->baseColorFactor));
  glUniform1f(glGetUniformLocation(shaderProgram, "material.metallicFactor"),
              material->metallicFactor);
  glUniform1f(glGetUniformLocation(shaderProgram, "material.roughnessFactor"),
              material->roughnessFactor);
  glUniform3fv(glGetUniformLocation(shaderProgram, "material.emissiveFactor"),
               1, glm::value_ptr(material->emissiveFactor));
  glUniform1f(glGetUniformLocation(shaderProgram, "material.emissiveStrength"),
              material->emissiveStrength);
}
