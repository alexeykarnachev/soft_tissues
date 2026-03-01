#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::system::transform {

Quaternion get_world_quaternion(entt::entity entity);
Vector3 get_world_position(entt::entity entity);
Matrix get_world_matrix(entt::entity entity);
Vector3 get_forward(entt::entity entity);
Vector3 get_right(entt::entity entity);

void set_forward(entt::entity entity, Vector3 forward);

}  // namespace soft_tissues::system::transform
