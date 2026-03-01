#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <nlohmann/json.hpp>

namespace soft_tissues::transform {

class Transform {
private:
    const entt::entity entity;

public:
    Vector3 _position = Vector3Zero();
    Vector3 _scale = Vector3One();
    Vector3 _rotation = Vector3Zero();

    Transform(entt::entity entity, Vector3 position);
    Transform(entt::entity entity, Vector3 position, Vector3 scale);
    Transform(entt::entity entity, Vector3 position, Vector3 scale, Vector3 rotation);

    Quaternion get_quaternion();
    Matrix get_matrix();
    Vector3 get_position();
    Vector3 get_forward();
    Vector3 get_right();

    void set_forward(Vector3 forward);

    void step(Vector3 step);
    void rotate_by_axis_angle(Vector3 axis, float angle);

    nlohmann::json to_json() const;
    static Transform from_json(entt::entity, const nlohmann::json &json_data);
};

}  // namespace soft_tissues::transform
