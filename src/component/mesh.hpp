#pragma once

#include "raylib/raylib.h"
#include <nlohmann/json.hpp>

namespace soft_tissues::component {

class MyMesh {

public:
    const std::string mesh_key;
    std::string material_pbr_key;
    Color constant_color;

    MyMesh(std::string mesh_key, std::string material_pbr_key);

    nlohmann::json to_json() const;
    static MyMesh from_json(const nlohmann::json &json_data);
};

}  // namespace soft_tissues::component
