#pragma once
#include "shader.hpp"
#include "texture.hpp"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

struct Vertex {
  glm::vec2 position;
  glm::vec2 texCoords;
  glm::vec4 color;

  Vertex(glm::vec2 position, glm::vec2 texCoords, glm::vec4 color)
      : position(position), texCoords(texCoords), color(color) {}
};

class SpriteBatch {
public:
  SpriteBatch(glm::vec2 windowSize);
  ~SpriteBatch();

  void UpdateCamera(glm::vec2 focalPoint, SDL_Rect tilemapBounds);

  void Draw(Texture *texture, glm::vec2 position,
            glm::vec2 scale = glm::vec2(1, 1), float rotation = 0.0f,
            glm::vec4 color = glm::vec4(1, 1, 1, 1),
            glm::vec4 srcRect = glm::vec4(0, 0, 0, 0),
            glm::vec2 flipPadding = glm::vec2(0, 0));

  void Draw(GLuint texture, glm::vec2 position,
            glm::vec2 scale = glm::vec2(1, 1), float rotation = 0.0f,
            glm::vec4 color = glm::vec4(1, 1, 1, 1),
            glm::vec4 srcRect = glm::vec4(0, 0, 0, 0),
            glm::vec2 flipPadding = glm::vec2(0, 0));

  void DrawRect(glm::vec4 destRect, glm::vec4 color = glm::vec4(1, 1, 1, 1));
  void Flush();

  void SetProjection(glm::vec2 windowSize);

  void SetTextureAndDimensions(GLuint texture, const int w, const int h);

private:
  std::vector<Vertex> vertices;
  GLuint vbo;

  std::vector<GLuint> indices;
  GLuint ebo;

  GLuint vao;

  Shader vertexShader;
  Shader fragmentShader;

  GLuint shaderProgram;

  GLuint texture;
  glm::ivec4 textureRect;
  GLuint textureUniform;

  glm::mat4 projection;
  GLuint projectionUniform;

  glm::mat4 view;
  GLuint viewUniform;

  glm::vec2 windowSize;
  glm::vec2 cameraPosition;
};
