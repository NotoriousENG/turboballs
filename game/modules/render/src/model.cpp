#include "model.hpp"

#include "tiny_gltf.h"
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
  } else {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Model: Unsupported file format: %s", ext.c_str());
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

  std::vector<std::shared_ptr<Material>> materials;

  // if there are no materials, create a placeholder material
  if (model.materials.empty()) {
    std::shared_ptr<Material> m = std::make_shared<Material>();
    m->baseColorFactor = glm::vec3(1.0f, 0.0f, 1.0f);
    m->metallicFactor = 1.0f;
    m->roughnessFactor = 1.0f;
    m->emissiveFactor = m->baseColorFactor;
    m->emissiveStrength = 1.0f;
    materials.push_back(m);
  }

  // load the materials
  for (const auto &mat : model.materials) {
    std::shared_ptr<Material> m = std::make_shared<Material>();
    // if no base color factor is present, set it to white
    if (mat.pbrMetallicRoughness.baseColorFactor.empty()) {
      m->baseColorFactor = glm::vec3(1.0f);
    } else {
      m->baseColorFactor =
          glm::vec3(mat.pbrMetallicRoughness.baseColorFactor[0],
                    mat.pbrMetallicRoughness.baseColorFactor[1],
                    mat.pbrMetallicRoughness.baseColorFactor[2]);
    }
    m->metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
    m->roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
    m->emissiveFactor = glm::vec3(mat.emissiveFactor[0], mat.emissiveFactor[1],
                                  mat.emissiveFactor[2]);
    m->emissiveStrength =
        mat.extensions.find("KHR_materials_emissive_strength") !=
                mat.extensions.end()
            ? mat.extensions.at("KHR_materials_emissive_strength")
                  .Get("emissiveStrength")
                  .Get<double>()
            : 0.0f;
    materials.push_back(m);
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

      // COLOR

      const auto &colorAccessor = model.accessors[p.attributes.at("COLOR_0")];
      const auto &colorBufferView = model.bufferViews[colorAccessor.bufferView];
      const auto &colorBuffer = model.buffers[colorBufferView.buffer];

      const unsigned short *colorData =
          reinterpret_cast<const unsigned short *>(
              &colorBuffer.data[colorAccessor.byteOffset +
                                colorBufferView.byteOffset]);

      const auto &colorType = colorAccessor.type;
      const auto &colorCompType = colorAccessor.componentType;
      const auto &colorCount = colorAccessor.count;
      const auto colorCompSize = colorAccessor.type;

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

      // Populate the vertices vector (pos, texCoords, normal, color)
      for (size_t i = 0; i < posCount; i++) {
        Vertex3D vertex;
        vertex.position =
            glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]);
        vertex.normal = glm::vec3(normalData[i * 3], normalData[i * 3 + 1],
                                  normalData[i * 3 + 2]);
        vertex.color = glm::vec4(
            colorData[i * 4] / 65535.0, colorData[i * 4 + 1] / 65535.0,
            colorData[i * 4 + 2] / 65535.0, colorData[i * 4 + 3] / 65535.0);
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

      // get the material id
      const auto materialId = p.material;
      if (materialId >= 0) {
        SDL_Log("Material id: %d", materialId);
        mesh->material = materials[materialId];
      } else {
        mesh->material = materials[0];
      }
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
