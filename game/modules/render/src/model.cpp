#include "model.hpp"

#include "tiny_obj_loader.h"
#include <SDL.h>

Model::Model(std::string path) {
  std::string dir = path.substr(0, path.find_last_of('/'));

  tinyobj::ObjReaderConfig reader_config;
  reader_config.mtl_search_path = dir; // Path to material files

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path, reader_config)) {
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

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {

    std::vector<Vertex3D> vertices;
    std::vector<GLuint> indices;

    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // create a vertex
        Vertex3D vertex;

        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

        vertex.position = glm::vec3(vx, vy, vz);

        // Check if `normal_index` is zero or positive. negative = no normal
        // data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

          vertex.normal = glm::vec3(nx, ny, nz);
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord
        // data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

          vertex.texCoords = glm::vec2(tx, ty);
        }

        // add the vertex to the vertices vector
        vertices.push_back(vertex);

        // Optional: vertex colors
        // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
        // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
        // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

        // Populate the index buffer
        indices.push_back(static_cast<GLuint>(index_offset + v));
      }
      index_offset += fv;
    }
    // load the materials
    for (size_t m = 0; m < materials.size(); m++) {
      SDL_Log("Material name: %s", materials[m].name.c_str());
      SDL_Log("Material id: %d", int(m));
      SDL_Log("Diffuse color: %f %f %f", materials[m].diffuse[0],
              materials[m].diffuse[1], materials[m].diffuse[2]);
    }

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices);
    meshes.push_back(mesh);
  }
}