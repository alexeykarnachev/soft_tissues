#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include <nlohmann/json.hpp>

namespace soft_tissues::mesh {

class MyMesh {

public:
    const entt::entity entity;
    const std::string mesh_key;
    std::string material_pbr_key;
    Color constant_color;

    MyMesh(entt::entity entity, std::string mesh_key, std::string material_pbr_key);

    nlohmann::json to_json() const;
    static MyMesh from_json(entt::entity entity, const nlohmann::json &json_data);
};

}  // namespace soft_tissues::mesh
