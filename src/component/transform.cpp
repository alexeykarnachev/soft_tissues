#include "transform.hpp"

#include "raylib/raymath.h"

namespace soft_tissues::transform {

Transform::Transform(entt::entity entity, Vector3 position)
    : entity(entity)
    , position(position)
    , scale(Vector3One())
    , rotation(QuaternionIdentity()) {}

Transform::Transform(entt::entity entity, Vector3 position, Vector3 scale)
    : entity(entity)
    , position(position)
    , scale(scale)
    , rotation(QuaternionIdentity()) {}

Transform::Transform(
    entt::entity entity, Vector3 position, Vector3 scale, Quaternion rotation
)
    : entity(entity)
    , position(position)
    , scale(scale)
    , rotation(rotation) {}

Matrix Transform::get_matrix() {
    Matrix r = QuaternionToMatrix(this->rotation);
    Matrix t = MatrixTranslate(this->position.x, this->position.y, this->position.z);
    Matrix s = MatrixScale(this->scale.x, this->scale.y, this->scale.z);

    Matrix mat = MatrixMultiply(s, MatrixMultiply(r, t));

    return mat;
}

Vector3 Transform::get_up() {
    return Vector3RotateByQuaternion({0.0, 1.0, 0.0}, this->rotation);
}

Vector3 Transform::get_forward() {
    return Vector3RotateByQuaternion({0.0, 0.0, -1.0}, this->rotation);
}

void Transform::translate(Vector3 vec) {
    this->position = Vector3Add(this->position, vec);
}

void Transform::rotate_x(float rad) {
    Quaternion q = QuaternionFromAxisAngle({1.0, 0.0, 0.0}, rad);
    this->rotation = QuaternionMultiply(this->rotation, q);
}

void Transform::rotate_y(float rad) {
    Quaternion q = QuaternionFromAxisAngle({0.0, 1.0, 0.0}, rad);
    this->rotation = QuaternionMultiply(this->rotation, q);
}

Vector3 Transform::apply_to_vector(Vector3 vec) {
    vec = Vector3RotateByQuaternion(vec, this->rotation);
    vec = Vector3Add(this->position, vec);
    return vec;
}

}  // namespace soft_tissues::transform
