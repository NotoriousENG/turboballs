#include "mesh-renderer.hpp"
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

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);
}

MeshRenderer::~MeshRenderer() {
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);
}

void MeshRenderer::Draw() {
  glUseProgram(this->shaderProgram);

  // set mat4 model
  GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
  glm::mat4 model = glm::mat4(1.0f);
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  // set mat4 view
  GLint viewLoc = glGetUniformLocation(this->shaderProgram, "view");
  // set camera position to (0, 2, 2) look at (0, 0, 0) with up vector (0, 1, 0)
  glm::mat4 view =
      glm::lookAt(glm::vec3(0, 2, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  // set mat4 projection
  GLint projectionLoc = glGetUniformLocation(this->shaderProgram, "projection");
  glm::mat4 projection =
      glm::perspective(glm::degrees(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glBindVertexArray(this->vao);

  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex3D),
               this->vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
               this->indices.data(), GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D),
                        (void *)offsetof(Vertex3D, position));
  glEnableVertexAttribArray(0);

  glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
  glUseProgram(0);
}
