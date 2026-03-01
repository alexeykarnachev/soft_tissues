#pragma once

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <nlohmann/json.hpp>

namespace soft_tissues::component {

struct Transform {
    Vector3 position = Vector3Zero();
    Vector3 scale = Vector3One();
    Vector3 rotation = Vector3Zero();

    Transform() = default;
    Transform(Vector3 position);
    Transform(Vector3 position, Vector3 scale);
    Transform(Vector3 position, Vector3 scale, Vector3 rotation);

    Quaternion get_local_quaternion() const;

    void step(Vector3 step);
    void rotate_by_axis_angle(Vector3 axis, float angle);

    nlohmann::json to_json() const;
    static Transform from_json(const nlohmann::json &json_data);
};

}  // namespace soft_tissues::component
