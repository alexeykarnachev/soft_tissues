#pragma once

#include "nlohmann/json.hpp"
#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::component {

struct MyMesh {
    std::string mesh_key;
    std::string material_pbr_key;
    Color constant_color;

    MyMesh(std::string mesh_key, std::string material_pbr_key);

    nlohmann::json to_json() const;
    static MyMesh from_json(const nlohmann::json &json_data);
};

}  // namespace soft_tissues::component
