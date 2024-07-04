#include "movement.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "globals.hpp"
#include "mode.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rcamera.h"

namespace soft_tissues::movement {

void update() {
    if (!mode::is_play()) return;

    Vector3 dir = Vector3Zero();
    if (IsKeyDown(KEY_W)) dir.z -= 1.0;
    if (IsKeyDown(KEY_S)) dir.z += 1.0;
    if (IsKeyDown(KEY_A)) dir.x -= 1.0;
    if (IsKeyDown(KEY_D)) dir.x += 1.0;

    float length = Vector3Length(dir);
    if (length <= EPSILON) return;

    dir = Vector3Scale(dir, 1.0 / length);
    Vector3 forward = GetCameraForward(&camera::CAMERA);
    Vector3 right = GetCameraRight(&camera::CAMERA);

    forward.y = 0.0;
    right.y = 0.0;

    forward = Vector3Scale(Vector3Normalize(forward), -dir.z);
    right = Vector3Scale(Vector3Normalize(right), dir.x);
    dir = Vector3Normalize(Vector3Add(forward, right));

    Vector3 step = Vector3Scale(dir, globals::DT * globals::PLAYER_MOVE_SPEED);

    auto player = globals::registry.view<component::Player>().front();
    auto &tr = globals::registry.get<component::Transform>(player);
    tr.position = Vector3Add(tr.position, step);

    camera::CAMERA.position = Vector3Add(camera::CAMERA.position, step);
    camera::CAMERA.target = Vector3Add(camera::CAMERA.target, step);
}

}  // namespace soft_tissues::movement
