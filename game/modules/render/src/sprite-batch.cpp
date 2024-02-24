#include "sprite-batch.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SpriteBatch::SpriteBatch(glm::vec2 windowSize) {
  this->vertexShader.LoadFromFile("assets/shaders/sprite.vert",
                                  GL_VERTEX_SHADER);
  this->fragmentShader.LoadFromFile("assets/shaders/sprite.frag",
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

  this->textureUniform =
      glGetUniformLocation(this->shaderProgram, "albedoTexture");
  this->texture = NULL;

  // Create and bind a VAO
  glGenVertexArrays(1, &this->vao);
  glBindVertexArray(this->vao);

  // Create and bind the EBO
  glGenBuffers(1, &this->ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);

  // Create and bind the VBO
  glGenBuffers(1, &this->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

  // Configure vertex attribute pointers in the VAO

  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);

  glEnableVertexAttribArray(1); // uv
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec2));

  glEnableVertexAttribArray(2); // color
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)sizeof(glm::vec4));

  glEnableVertexAttribArray(3); // rotation
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (GLvoid *)(2 * sizeof(glm::vec2) + sizeof(glm::vec4)));

  // Unbind the VAO and VBO
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  this->projectionUniform =
      glGetUniformLocation(this->shaderProgram, "projection");
  this->viewUniform = glGetUniformLocation(this->shaderProgram, "view");

  this->SetProjection(windowSize);
}

SpriteBatch::~SpriteBatch() {
  glDeleteBuffers(1, &this->vbo);
  glDeleteBuffers(1, &this->ebo);
  glDeleteVertexArrays(1, &this->vao);

  glDeleteProgram(this->shaderProgram);
}

void SpriteBatch::UpdateCamera(glm::vec2 focalPoint, SDL_Rect tilemapBounds) {
  // clamp the focal point to the tilemap bounds
  focalPoint.x =
      glm::clamp(static_cast<int>(focalPoint.x),
                 static_cast<int>(tilemapBounds.x + this->windowSize.x / 2.0f),
                 static_cast<int>(tilemapBounds.x + tilemapBounds.w -
                                  this->windowSize.x / 2.0f));
  focalPoint.y =
      glm::clamp(static_cast<int>(focalPoint.y),
                 static_cast<int>(tilemapBounds.y + this->windowSize.y / 2.0f),
                 static_cast<int>(tilemapBounds.y + tilemapBounds.h -
                                  this->windowSize.y / 2.0f));

  // update the camera position
  this->cameraPosition = focalPoint;
  // update the view, so that the target is always in the center of the screen
  this->view = glm::translate(
      glm::mat4(1.0f), glm::vec3(-focalPoint.x + this->windowSize.x / 2,
                                 -focalPoint.y + this->windowSize.y / 2, 0.0f));
}

void SpriteBatch::Draw(Texture *texture, glm::vec2 position, glm::vec2 scale,
                       float rotation, glm::vec4 color, glm::vec4 srcRect,
                       glm::vec2 flipPadding) {

  if (this->texture != texture->GetGLTexture()) {
    this->Flush();
    this->texture = texture->GetGLTexture();
  }

  this->textureRect =
      texture != nullptr ? texture->GetTextureRect() : glm::ivec4(0, 0, 1, 1);
  if (srcRect == glm::vec4(0, 0, 0, 0)) {
    srcRect = this->textureRect;
  }
  if (flipPadding == glm::vec2(0, 0)) {
    flipPadding = glm::vec2(textureRect.z, textureRect.w);
  }

  this->Draw(this->texture, position, scale, rotation, color, srcRect,
             flipPadding);
}

void SpriteBatch::Draw(GLuint texture, glm::vec2 position, glm::vec2 scale,
                       float rotation, glm::vec4 color, glm::vec4 srcRect,
                       glm::vec2 flipPadding) {
  glm::vec2 center(position.x + (srcRect.z * scale.x) * 0.5f,
                   position.y + (srcRect.w * scale.y) * 0.5f);

  glm::vec2 scaledTopLeft(-srcRect.z * scale.x * 0.5f,
                          -srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledTopRight(srcRect.z * scale.x * 0.5f,
                           -srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledBottomLeft(-srcRect.z * scale.x * 0.5f,
                             srcRect.w * scale.y * 0.5f);
  glm::vec2 scaledBottomRight(srcRect.z * scale.x * 0.5f,
                              srcRect.w * scale.y * 0.5f);

  // Rotate the vertices
  const glm::mat2 rotationMatrix(glm::cos(rotation), -glm::sin(rotation),
                                 glm::sin(rotation), glm::cos(rotation));
  scaledTopLeft = rotationMatrix * scaledTopLeft + center;
  scaledTopRight = rotationMatrix * scaledTopRight + center;
  scaledBottomLeft = rotationMatrix * scaledBottomLeft + center;
  scaledBottomRight = rotationMatrix * scaledBottomRight + center;

  const float textureWidth = this->textureRect.z;
  const float textureHeight = this->textureRect.w;

  const glm::vec2 uvTopLeft(srcRect.x / textureWidth,
                            srcRect.y / textureHeight);
  const glm::vec2 uvTopRight((srcRect.x + srcRect.z) / textureWidth,
                             srcRect.y / textureHeight);
  const glm::vec2 uvBottomLeft(srcRect.x / textureWidth,
                               (srcRect.y + srcRect.w) / textureHeight);
  const glm::vec2 uvBottomRight((srcRect.x + srcRect.z) / textureWidth,
                                (srcRect.y + srcRect.w) / textureHeight);

  // dumb but works
  if (scale.y < 0) {
    scaledTopRight.y += flipPadding.y;
    scaledBottomRight.y += flipPadding.y;
    scaledBottomLeft.y += flipPadding.y;
    scaledTopLeft.y += flipPadding.y;
  }
  if (scale.x < 0) {
    scaledBottomLeft.x += flipPadding.x;
    scaledBottomRight.x += flipPadding.x;
    scaledTopRight.x += flipPadding.x;
    scaledTopLeft.x += flipPadding.x;
  }

  // Calculate the vertex index offset for the current sprite
  const int vertexIndexOffset = this->vertices.size();

  // Add vertices to the list
  this->vertices.push_back(Vertex(scaledTopLeft, uvTopLeft, color));
  this->vertices.push_back(Vertex(scaledTopRight, uvTopRight, color));
  this->vertices.push_back(Vertex(scaledBottomLeft, uvBottomLeft, color));
  this->vertices.push_back(Vertex(scaledBottomRight, uvBottomRight, color));

  // Add indices for the two triangles forming the quad for the current sprite
  // Add indices offset by vertexIndexOffset
  this->indices.push_back(vertexIndexOffset + 0);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 3);
}

void SpriteBatch::DrawRect(glm::vec4 destRect, glm::vec4 color) {

  if (this->texture != NULL) {
    this->Flush();
    this->texture = NULL;
  }

  const int vertexIndexOffset = this->vertices.size();

  const auto topLeft = glm::vec2(destRect.x, destRect.y);
  const auto topRight = glm::vec2(destRect.x + destRect.z, destRect.y);
  const auto bottomLeft = glm::vec2(destRect.x, destRect.y + destRect.w);
  const auto bottomRight =
      glm::vec2(destRect.x + destRect.z, destRect.y + destRect.w);

  const auto uvTopLeft = glm::vec2(0, 0);
  const auto uvTopRight = glm::vec2(1, 0);
  const auto uvBottomLeft = glm::vec2(0, 1);
  const auto uvBottomRight = glm::vec2(1, 1);

  this->vertices.push_back(Vertex(topLeft, uvTopLeft, color));
  this->vertices.push_back(Vertex(topRight, uvTopRight, color));
  this->vertices.push_back(Vertex(bottomLeft, uvBottomLeft, color));
  this->vertices.push_back(Vertex(bottomRight, uvBottomRight, color));

  // Add indices for the two triangles forming the quad for the current sprite
  // Add indices offset by vertexIndexOffset
  this->indices.push_back(vertexIndexOffset + 0);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 2);
  this->indices.push_back(vertexIndexOffset + 1);
  this->indices.push_back(vertexIndexOffset + 3);
}

void SpriteBatch::Flush() {
  if (this->vertices.size() == 0) {
    return;
  }

  glUseProgram(this->shaderProgram);
  glBindVertexArray(this->vao); // Bind the VAO

  GLuint whiteTexture;

  if (this->texture == NULL) {
    // If the texture is undefined, bind a 1x1 white texture
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    unsigned char whitePixel[] = {255, 255, 255, 255}; // White color
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 whitePixel);
    glUniform1i(this->textureUniform, 0);
  } else {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUniform1i(this->textureUniform, 0);
  }

  // Upload the vertex data to the GPU
  glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
  glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
               &this->vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Upload the index data to the GPU
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
               &this->indices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUniformMatrix4fv(this->projectionUniform, 1, GL_FALSE,
                     glm::value_ptr(this->projection));

  glUniformMatrix4fv(this->viewUniform, 1, GL_FALSE,
                     glm::value_ptr(this->view));

  glEnableVertexAttribArray(0); // position
  glEnableVertexAttribArray(1); // uv
  glEnableVertexAttribArray(2); // color
  glEnableVertexAttribArray(3); // rotation

  glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

  glDisableVertexAttribArray(0); // position
  glDisableVertexAttribArray(1); // uv
  glDisableVertexAttribArray(2); // color
  glDisableVertexAttribArray(3); // rotation

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind the EBO
  glBindVertexArray(0);                     // Unbind the VAO
  this->vertices.clear();
  this->indices.clear();
  if (this->texture == NULL) {
    // Delete the temporary white texture if it was created
    glDeleteTextures(1, &whiteTexture);
  }
}

void SpriteBatch::SetProjection(glm::vec2 windowSize) {
  // create a projection matrix that will make the screen coordinates (0,0)
  // top left to (windowSize.x, windowSize.y) bottom right
  this->windowSize = windowSize;
  this->projection =
      glm::ortho(0.0f, this->windowSize.x, this->windowSize.y, 0.0f);
}

void SpriteBatch::SetTextureAndDimensions(GLuint texture, const int w,
                                          const int h) {
  this->texture = texture;
  this->textureRect = glm::ivec4(0, 0, w, h);
}
