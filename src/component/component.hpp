#pragma once

#include "entt/entity/fwd.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "transform.hpp"

namespace soft_tissues::component {

using Transform = transform::Transform;
using Light = light::Light;
using MyMesh = mesh::MyMesh;

struct Player {};
struct Flashlight {};
struct Parent {
    entt::entity entity;
};

}  // namespace soft_tissues::component
