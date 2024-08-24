#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::mesh {

class MyMesh {
public:
    const entt::entity entity;

    const std::string mesh_key;
    std::string material_pbr_key;

    Color constant_color;

    MyMesh(entt::entity entity, std::string mesh_key, std::string material_pbr_key);
};

}  // namespace soft_tissues::mesh
