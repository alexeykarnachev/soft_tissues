#include "movement.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "mode.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rcamera.h"
#include <cstdio>

namespace soft_tissues::movement {

void update_translation() {
    Vector3 dir = Vector3Zero();
    if (IsKeyDown(KEY_W)) dir.z -= 1.0;
    if (IsKeyDown(KEY_S)) dir.z += 1.0;
    if (IsKeyDown(KEY_A)) dir.x -= 1.0;
    if (IsKeyDown(KEY_D)) dir.x += 1.0;

    float length = Vector3Length(dir);
    if (length <= EPSILON) return;

    auto player = globals::registry.view<component::Player>().front();
    auto &tr = globals::registry.get<component::Transform>(player);

    Vector3 forward = tr.get_forward();
    Vector3 right = tr.get_right();

    forward.y = 0.0;
    right.y = 0.0;

    dir = Vector3Scale(dir, 1.0 / length);
    forward = Vector3Scale(Vector3Normalize(forward), -dir.z);
    right = Vector3Scale(Vector3Normalize(right), dir.x);
    dir = Vector3Normalize(Vector3Add(forward, right));

    Vector3 step = Vector3Scale(dir, globals::FRAME_DT * globals::PLAYER_MOVEMENT_SPEED);

    tr.position = Vector3Add(tr.position, step);
}

void update_rotation() {
    Vector2 mouse_delta = GetMouseDelta();
    mouse_delta = Vector2Scale(mouse_delta, globals::PLAYER_CAMERA_SENSETIVITY);

    float yaw_delta = -mouse_delta.x;
    float pitch_delta = -mouse_delta.y;
    if (yaw_delta == 0 && pitch_delta == 0) return;

    auto player = globals::registry.view<component::Player>().front();
    auto &tr = globals::registry.get<component::Transform>(player);

    float pitch = tr.rotation.x + pitch_delta;
    pitch = Clamp(pitch, -0.5 * PI + 0.025, 0.5 * PI - 0.025);
    float yaw = tr.rotation.y + yaw_delta;

    tr.rotation.x = pitch;
    tr.rotation.y = yaw;
}

void update() {
    if (!mode::is_play()) return;

    update_translation();
    update_rotation();
}

}  // namespace soft_tissues::movement
