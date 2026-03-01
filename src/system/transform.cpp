#include "transform.hpp"

#include "../component/component.hpp"
#include "../globals.hpp"
#include "raylib/raymath.h"

namespace soft_tissues::system::transform {

Quaternion get_world_quaternion(entt::entity entity) {
    auto &tr = globals::registry.get<component::Transform>(entity);

    Quaternion parent_q = QuaternionIdentity();
    auto *parent = globals::registry.try_get<component::Parent>(entity);
    if (parent != nullptr) {
        parent_q = get_world_quaternion(parent->entity);
    }

    auto my_q = tr.get_local_quaternion();
    return QuaternionMultiply(parent_q, my_q);
}

Vector3 get_world_position(entt::entity entity) {
    auto &tr = globals::registry.get<component::Transform>(entity);

    Vector3 parent_pos = Vector3Zero();
    auto *parent = globals::registry.try_get<component::Parent>(entity);
    if (parent != nullptr) {
        parent_pos = get_world_position(parent->entity);
    }

    return Vector3Add(parent_pos, tr.position);
}

Matrix get_world_matrix(entt::entity entity) {
    Vector3 pos = get_world_position(entity);
    Quaternion q = get_world_quaternion(entity);

    Matrix t = MatrixTranslate(pos.x, pos.y, pos.z);
    Matrix r = QuaternionToMatrix(q);

    return MatrixMultiply(r, t);
}

Vector3 get_forward(entt::entity entity) {
    return Vector3RotateByQuaternion({0.0, 0.0, -1.0}, get_world_quaternion(entity));
}

Vector3 get_right(entt::entity entity) {
    return Vector3RotateByQuaternion({1.0, 0.0, 0.0}, get_world_quaternion(entity));
}

void set_forward(entt::entity entity, Vector3 forward) {
    Vector3 old_forward = get_forward(entity);
    auto &tr = globals::registry.get<component::Transform>(entity);
    auto new_q = QuaternionFromVector3ToVector3(old_forward, forward);
    auto my_q = tr.get_local_quaternion();
    auto q = QuaternionMultiply(new_q, my_q);
    tr.rotation = QuaternionToEuler(q);
}

}  // namespace soft_tissues::system::transform
