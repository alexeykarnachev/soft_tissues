#include "transform.hpp"

#include "../globals.hpp"
#include "../serializers.hpp"
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

Matrix Transform::get_matrix() {
    Vector3 pos = this->get_position();
    Quaternion q = this->get_quaternion();

    Matrix t = MatrixTranslate(pos.x, pos.y, pos.z);
    Matrix r = QuaternionToMatrix(q);

    Matrix mat = MatrixMultiply(r, t);

    return mat;
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

void Transform::set_forward(Vector3 forward) {
    Vector3 old_forward = this->get_forward();
    auto new_q = QuaternionFromVector3ToVector3(old_forward, forward);
    auto my_q = QuaternionFromEuler(
        this->_rotation.x, this->_rotation.y, this->_rotation.z
    );
    auto q = QuaternionMultiply(new_q, my_q);
    this->_rotation = QuaternionToEuler(q);
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

nlohmann::json Transform::to_json() const {
    nlohmann::json json;

    json["position"] = this->_position;
    json["scale"] = this->_scale;
    json["rotation"] = this->_rotation;

    return json;
}

Transform Transform::from_json(entt::entity entity, const nlohmann::json &json_data) {
    Vector3 position = json_data["position"].get<Vector3>();
    Vector3 scale = json_data["scale"].get<Vector3>();
    Vector3 rotation = json_data["rotation"].get<Vector3>();

    return Transform(entity, position, scale, rotation);
}

}  // namespace soft_tissues::transform
