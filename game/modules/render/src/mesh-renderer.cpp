#include "mesh-renderer.hpp"
#include "tiny_obj_loader.h"
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
  loadModel();

  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glGenVertexArrays(1, &vao);

  glUseProgram(this->shaderProgram);

  GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
  model = glm::mat4(1.0f);
  // scale the model
  model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
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
      glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
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
  // rotate the model on the y axis 90 degrees
  model = glm::rotate(model, glm::radians(1.0f), glm::vec3(0, 1, 0));
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

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

void MeshRenderer::loadModel() {
  //  load obj file
  std::string inputfile = "assets/models/12221_Cat_v1_l3.obj";
  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = "./"; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(inputfile, reader_config)) {
    if (!reader.Error().empty()) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TinyObjReader: %s",
                   reader.Error().c_str());
    }
    return;
  }

  if (!reader.Warning().empty()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TinyObjReader: %s",
                reader.Warning().c_str());
  }

  auto &attrib = reader.GetAttrib();
  auto &shapes = reader.GetShapes();
  auto &materials = reader.GetMaterials();

  this->vertices.clear();
  this->indices.clear();

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        this->vertices.push_back(Vertex3D(vx, vy, vz));
        SDL_Log("Vertex: %f, %f, %f", vx, vy, vz);

        // Check if `normal_index` is zero or positive. negative = no normal
        // data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord
        // data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
        }

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

        // Populate the index buffer
        this->indices.push_back(static_cast<GLuint>(index_offset + v));
        SDL_Log("Index: %d", index_offset + v);
      }
      index_offset += fv;

      // per-face material
      shapes[s].mesh.material_ids[f];
    }
  }
}
