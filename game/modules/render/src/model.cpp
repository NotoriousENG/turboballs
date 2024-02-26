#include "model.hpp"

#include "tiny_gltf.h"
#include "tiny_obj_loader.h"
#include <SDL.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

Model::Model(std::string path) {
  // get file extension
  std::string ext = path.substr(path.find_last_of('.') + 1);
  for (auto &c : ext) {
    c = tolower(c);
  }

  SDL_Log("File extension: %s", ext.c_str());

  if (ext == "gltf" || ext == "glb") {
    loadGLTF(path);
  } else if (ext == "obj") {
    loadObj(path);
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Model: Unsupported file format: %s", ext.c_str());
  }
}

void Model::loadObj(std::string path) {
  SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Warning: prefer glTF over obj");

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

void Model::loadGLTF(std::string path) {
  const bool isBinary = (path.substr(path.find_last_of('.') + 1) == "glb");

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = isBinary ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
                      : loader.LoadASCIIFromFile(&model, &err, &warn, path);

  if (!warn.empty()) {
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "TinyGLTF: %s", warn.c_str());
  }

  if (!err.empty()) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TinyGLTF: %s", err.c_str());
  }

  if (!ret) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "TinyGLTF: Failed to load glTF");
    return;
  }

  // Loop through the model and load the meshes
  for (const auto &m : model.meshes) {
    for (const auto &p : m.primitives) {
      std::vector<Vertex3D> vertices;
      std::vector<GLuint> indices;

      // Get the vertex data

      // POSITION

      const auto &posAccessor = model.accessors[p.attributes.at("POSITION")];
      const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
      const auto &posBuffer = model.buffers[posBufferView.buffer];

      const float *posData = reinterpret_cast<const float *>(
          &posBuffer.data[posAccessor.byteOffset + posBufferView.byteOffset]);

      const auto &posType = posAccessor.type;
      const auto &posCompType = posAccessor.componentType;
      const auto &posCount = posAccessor.count;
      const auto &posCompSize = posAccessor.type;

      // NORMAL

      const auto &normalAccessor = model.accessors[p.attributes.at("NORMAL")];
      const auto &normalBufferView =
          model.bufferViews[normalAccessor.bufferView];
      const auto &normalBuffer = model.buffers[normalBufferView.buffer];

      const float *normalData = reinterpret_cast<const float *>(
          &normalBuffer
               .data[normalAccessor.byteOffset + normalBufferView.byteOffset]);

      const auto &normalType = normalAccessor.type;
      const auto &normalCompType = normalAccessor.componentType;
      const auto &normalCount = normalAccessor.count;
      const auto &normalCompSize = normalAccessor.type;

      // TEXCOORD_0

      const auto &texAccessor = model.accessors[p.attributes.at("TEXCOORD_0")];
      const auto &texBufferView = model.bufferViews[texAccessor.bufferView];
      const auto &texBuffer = model.buffers[texBufferView.buffer];

      const float *texData = reinterpret_cast<const float *>(
          &texBuffer.data[texAccessor.byteOffset + texBufferView.byteOffset]);

      const auto &texType = texAccessor.type;
      const auto &texCompType = texAccessor.componentType;
      const auto &texCount = texAccessor.count;
      const auto &texCompSize = texAccessor.type;

      // Populate the vertices vector (pos, texCoords, normal)
      for (size_t i = 0; i < posCount; i++) {
        Vertex3D vertex;
        vertex.position =
            glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
        vertex.normal = glm::vec3(normalData[i * 3], normalData[i * 3 + 1],
                                  normalData[i * 3 + 2]);
        vertex.texCoords = glm::vec2(texData[i * 2], texData[i * 2 + 1]);

        vertices.push_back(vertex);
      }

      // Get the index data
      const auto &indexAccessor = model.accessors[p.indices];
      const auto &indexBufferView = model.bufferViews[indexAccessor.bufferView];
      const auto &indexBuffer = model.buffers[indexBufferView.buffer];

      const auto indexType = indexAccessor.componentType;
      const auto indexCount = indexAccessor.count;

      const void *indexData =
          &indexBuffer
               .data[indexAccessor.byteOffset + indexBufferView.byteOffset];

      // Populate the indices vector
      // gltf indices can be of type UNSIGNED_INT, UNSIGNED_SHORT or
      // UNSIGNED_BYTE, without this branching stack smashing occurs
      for (size_t i = 0; i < indexCount; i++) {
        if (indexType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT) {
          const auto *data = reinterpret_cast<const uint32_t *>(indexData);
          indices.push_back(data[i]);
        } else if (indexType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT) {
          const auto *data = reinterpret_cast<const uint16_t *>(indexData);
          indices.push_back(data[i]);
        } else if (indexType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE) {
          const auto *data = reinterpret_cast<const uint8_t *>(indexData);
          indices.push_back(data[i]);
        }
      }

      std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(vertices, indices);
      meshes.push_back(mesh);
    }
  }

  // get nodes
  for (const auto &node : model.nodes) {
    SDL_Log("Node name: %s", node.name.c_str());

    const auto meshId = node.mesh;

    // get the mesh id
    if (meshId >= 0) {
      SDL_Log("Mesh id: %d", meshId);
    } else {
      continue;
    }

    glm::mat4 model = glm::mat4(1.0f);

    // get the translation, rotation and scale (if any)
    if (!node.translation.empty()) {
      model = glm::translate(model,
                             glm::vec3(node.translation[0], node.translation[1],
                                       node.translation[2]));
    }
    if (!node.rotation.empty()) {
      glm::quat q = glm::quat(node.rotation[3], node.rotation[0],
                              node.rotation[1], node.rotation[2]);
      model = model * glm::mat4_cast(q);
    }
    if (!node.scale.empty()) {
      model = glm::scale(
          model, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }

    // add the model matrix to the mesh
    meshes[meshId]->model = model;
  }
}
