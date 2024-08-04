#include "transform.hpp"

#include "../globals.hpp"
#include "component.hpp"
#include "raylib/raymath.h"
#include <cstdio>

namespace soft_tissues::transform {

Transform::Transform(entt::entity entity, Vector3 position)
    : entity(entity)
    , _position(position)
    , _scale(Vector3One())
    , _rotation(Vector3Zero()) {}

Transform::Transform(entt::entity entity, Vector3 position, Vector3 scale)
    : entity(entity)
    , _position(position)
    , _scale(scale)
    , _rotation(Vector3Zero()) {}

Transform::Transform(
    entt::entity entity, Vector3 position, Vector3 scale, Vector3 rotation
)
    : entity(entity)
    , _position(position)
    , _scale(scale)
    , _rotation(rotation) {}

Quaternion Transform::get_quaternion() {
    auto parent = globals::registry.try_get<component::Parent>(this->entity);
    Quaternion parent_q = QuaternionIdentity();
    if (parent != NULL) {
        auto parent_tr = globals::registry.get<component::Transform>(parent->entity);
        parent_q = parent_tr.get_quaternion();
    }

    auto my_q = QuaternionFromEuler(
        this->_rotation.x, this->_rotation.y, this->_rotation.z
    );
    auto q = QuaternionMultiply(parent_q, my_q);
    return q;
}

Vector3 Transform::get_position() {
    auto parent = globals::registry.try_get<component::Parent>(this->entity);
    Vector3 parent_pos = Vector3Zero();
    if (parent != NULL) {
        auto parent_tr = globals::registry.get<component::Transform>(parent->entity);
        parent_pos = parent_tr.get_position();
    }

    auto position = Vector3Add(parent_pos, this->_position);
    return position;
}

Vector3 Transform::get_forward() {
    return Vector3RotateByQuaternion({0.0, 0.0, -1.0}, this->get_quaternion());
}

Vector3 Transform::get_right() {
    return Vector3RotateByQuaternion({1.0, 0.0, 0.0}, this->get_quaternion());
}

void Transform::step(Vector3 step) {
    this->_position = Vector3Add(this->_position, step);
}

void Transform::rotate_by_axis_angle(Vector3 axis, float angle) {
    auto new_q = QuaternionFromAxisAngle(axis, angle);
    auto my_q = QuaternionFromEuler(
        this->_rotation.x, this->_rotation.y, this->_rotation.z
    );
    auto q = QuaternionMultiply(new_q, my_q);
    this->_rotation = QuaternionToEuler(q);
}

}  // namespace soft_tissues::transform
