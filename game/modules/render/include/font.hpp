#pragma once
#include <ft2build.h>

#include FT_FREETYPE_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "sprite-batch.hpp"

#include <unordered_map>

#define ASCII_OFFSET 32
#define ASCII_MAX 126
#define ASCII_EXTENDED_MAX 255

typedef unsigned int uint;

struct Glyph {
  glm::vec2 offset;
  glm::ivec2 size;
  glm::ivec2 texCoords;
  glm::vec2 advance;
};

class Font {
public:
  Font(const char *path, int size);
  void RenderText(SpriteBatch *renderer, const char *text, glm::vec2 position,
                  glm::vec2 scale, glm::vec4 color,
                  glm::vec2 *outDims = nullptr, float wrapWidth = -1);

  int GetFontSize() { return this->fontSize; }

  // get text rect
  glm::vec2 GetTextDimensions(const char *text);

private:
  int fontSize;
  long max_height;

  std::unordered_map<char, Glyph> glyphs;

  GLuint tex;
  int texDim = 512;
};