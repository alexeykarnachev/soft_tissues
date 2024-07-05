#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::transform {

class Transform {
private:
    const entt::entity entity;

public:
    Vector3 position = Vector3Zero();
    Vector3 scale = Vector3One();
    Vector3 rotation = Vector3Zero();

    Transform(entt::entity entity, Vector3 position);
    Transform(entt::entity entity, Vector3 position, Vector3 scale);
    Transform(entt::entity entity, Vector3 position, Vector3 scale, Vector3 rotation);

    Quaternion get_quaternion();
    Vector3 get_forward();
    Vector3 get_right();
};

}  // namespace soft_tissues::transform
