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
    Quaternion rotation = QuaternionIdentity();

    Transform(entt::entity entity, Vector3 position);
    Transform(entt::entity entity, Vector3 position, Vector3 Scale);
    Transform(entt::entity entity, Vector3 position, Vector3 Scale, Quaternion rotation);

    Matrix get_matrix();

    Vector3 get_up();
    Vector3 get_forward();

    void translate(Vector3 vec);

    void rotate_x(float rad);
    void rotate_y(float rad);

    Vector3 apply_to_vector(Vector3 vec);
};

}  // namespace soft_tissues::transform
