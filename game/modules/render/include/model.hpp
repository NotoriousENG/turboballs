#pragma once

#include "mesh.hpp"

#include <memory>
#include <vector>

class Model {
public:
  Model(std::string path);

  const std::vector<std::shared_ptr<Mesh>> getMeshes() { return meshes; }

private:
  std::vector<std::shared_ptr<Mesh>> meshes;
};