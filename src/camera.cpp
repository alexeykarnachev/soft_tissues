#include "camera.hpp"

#include "component/component.hpp"
#include "component/transform.hpp"
#include "globals.hpp"
#include "mode.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rcamera.h"
#include <cstdio>

namespace soft_tissues::camera {

Camera3D CAMERA = {
    .position = {0.0, 1.0, 0.0},
    .target = {0.0, 0.0, -1.0},
    .up = {0.0, 1.0, 0.0},
    .fovy = 70.0,
    .projection = CAMERA_PERSPECTIVE,
};

static void update_editor_mode() {
    static const float rot_speed = 0.003;
    static const float move_speed = 0.01;
    static const float zoom_speed = 1.0;

    bool is_mmb_down = IsMouseButtonDown(2);
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    bool is_moving = is_mmb_down && is_shift_down;
    bool is_rotating = is_mmb_down && !is_shift_down;

    // move
    if (is_moving) {
        CameraMoveRight(&CAMERA, -move_speed * mouse_delta.x, true);

        Vector3 up_norm = Vector3Normalize(CAMERA.up);
        Vector3 up = Vector3Scale(up_norm, move_speed * mouse_delta.y);

        CAMERA.position = Vector3Add(CAMERA.position, up);
        CAMERA.target = Vector3Add(CAMERA.target, up);
    }

    // rotate
    if (is_rotating) {
        CameraYaw(&CAMERA, -rot_speed * mouse_delta.x, true);
        CameraPitch(&CAMERA, rot_speed * mouse_delta.y, true, true, false);
    }

    // zoom
    CameraMoveToTarget(&CAMERA, -GetMouseWheelMove() * zoom_speed);
}

static void update_first_person_mode() {
    static float rot_speed = 0.003;

    auto player = globals::registry.view<component::Player>().front();
    auto tr = globals::registry.get<component::Transform>(player);

    CAMERA.position = tr.position;
    CAMERA.position.y += globals::PLAYER_HEIGHT;

    Vector2 mouse_delta = GetMouseDelta();
    CameraYaw(&CAMERA, -rot_speed * mouse_delta.x, false);
    CameraPitch(&CAMERA, -rot_speed * mouse_delta.y, true, false, false);

    Vector3 forward = GetCameraForward(&CAMERA);
    Vector3 target = Vector3Add(CAMERA.position, forward);
    CAMERA.target = target;
}

void update() {
    if (GetTime() < 0.2) return;

    mode::Mode mode = mode::get_mode();
    switch (mode) {
        case mode::Mode::EDITOR: update_editor_mode(); break;
        case mode::Mode::PLAY: update_first_person_mode(); break;
    }
}

}  // namespace soft_tissues::camera
