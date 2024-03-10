#pragma once

#include "mesh.hpp"

#include <string>
#include <memory>
#include <vector>

class Model {
public:
  Model(std::string path);

  const std::vector<std::shared_ptr<Mesh>> getMeshes() { return meshes; }

private:
  void loadObj(std::string path);
  void loadGLTF(std::string path);
  std::vector<std::shared_ptr<Mesh>> meshes;
};