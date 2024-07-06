#include "camera.hpp"

#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rcamera.h"
#include <cstdio>

namespace soft_tissues::camera {

Camera3D CAMERA = {
    .position = {5.0, 5.0, 5.0},
    .target = {0.0, 0.0, 0.0},
    .up = {0.0, 1.0, 0.0},
    .fovy = 80.0,
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

void update() {
    switch (globals::GAME_STATE) {
        case globals::GameState::EDITOR: update_editor_mode(); break;
    }
}

}  // namespace soft_tissues::camera
