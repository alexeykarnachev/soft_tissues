#pragma once

#include "../pbr.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::mesh {

class MyMesh {
public:
    const entt::entity entity;

    Mesh mesh;
    pbr::MaterialPBR material;
    Color constant_color;

    MyMesh(entt::entity entity, Mesh mesh, pbr::MaterialPBR material);
};

}  // namespace soft_tissues::mesh
