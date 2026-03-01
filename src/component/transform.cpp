#include "transform.hpp"

#include "serializers.hpp"
#include "raylib/raymath.h"

namespace soft_tissues::component {

Transform::Transform(Vector3 position)
    : position(position)
    , scale(Vector3One())
    , rotation(Vector3Zero()) {}

Transform::Transform(Vector3 position, Vector3 scale)
    : position(position)
    , scale(scale)
    , rotation(Vector3Zero()) {}

Transform::Transform(Vector3 position, Vector3 scale, Vector3 rotation)
    : position(position)
    , scale(scale)
    , rotation(rotation) {}

Quaternion Transform::get_local_quaternion() const {
    return QuaternionFromEuler(this->rotation.x, this->rotation.y, this->rotation.z);
}

nlohmann::json Transform::to_json() const {
    nlohmann::json json;

    json["position"] = this->position;
    json["scale"] = this->scale;
    json["rotation"] = this->rotation;

    return json;
}

Transform Transform::from_json(const nlohmann::json &json_data) {
    Vector3 position = json_data["position"].get<Vector3>();
    Vector3 scale = json_data["scale"].get<Vector3>();
    Vector3 rotation = json_data["rotation"].get<Vector3>();

    return Transform(position, scale, rotation);
}

}  // namespace soft_tissues::component
