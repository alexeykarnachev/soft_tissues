#include "transform.hpp"

#include "raylib/raymath.h"

namespace soft_tissues::transform {

Transform::Transform(entt::entity entity, Vector3 position)
    : entity(entity)
    , position(position)
    , scale(Vector3One())
    , rotation(Vector3Zero()) {}

Transform::Transform(entt::entity entity, Vector3 position, Vector3 scale)
    : entity(entity)
    , position(position)
    , scale(scale)
    , rotation(Vector3Zero()) {}

Transform::Transform(
    entt::entity entity, Vector3 position, Vector3 scale, Vector3 rotation
)
    : entity(entity)
    , position(position)
    , scale(scale)
    , rotation(rotation) {}

Quaternion Transform::get_quaternion() {
    return QuaternionFromEuler(this->rotation.x, this->rotation.y, this->rotation.z);
}

Vector3 Transform::get_forward() {
    return Vector3RotateByQuaternion({0.0, 0.0, -1.0}, this->get_quaternion());
}

Vector3 Transform::get_right() {
    return Vector3RotateByQuaternion({1.0, 0.0, 0.0}, this->get_quaternion());
}

}  // namespace soft_tissues::transform
