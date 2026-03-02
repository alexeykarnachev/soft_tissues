#include "camera.hpp"

#include "component/component.hpp"
#include "gameplay_config.hpp"
#include "globals.hpp"
#include "system/transform.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rcamera.h"

namespace soft_tissues::system::camera {

Camera3D CAMERA = {
    .position = {0.0, 1.0, 0.0},
    .target = {0.0, 0.0, -1.0},
    .up = {0.0, 1.0, 0.0},
    .fovy = gameplay_config::PLAYER_FOV,
    .projection = CAMERA_PERSPECTIVE,
};

static void update_editor_mode() {
    static const float ROT_SPEED = 0.003;
    static const float MOVE_SPEED = 0.01;
    static const float ZOOM_SPEED = 1.0;

    bool is_mmb_down = IsMouseButtonDown(MOUSE_BUTTON_MIDDLE);
    bool is_shift_down = IsKeyDown(KEY_LEFT_SHIFT);
    Vector2 mouse_delta = GetMouseDelta();

    bool is_moving = is_mmb_down && is_shift_down;
    bool is_rotating = is_mmb_down && !is_shift_down;

    // move
    if (is_moving) {
        CameraMoveRight(&CAMERA, -MOVE_SPEED * mouse_delta.x, true);

        // camera basis
        auto z = GetCameraForward(&CAMERA);
        auto x = Vector3Normalize(Vector3CrossProduct(z, {0.0, 1.0, 0.0}));
        auto y = Vector3Normalize(Vector3CrossProduct(x, z));

        Vector3 up = Vector3Scale(y, MOVE_SPEED * mouse_delta.y);

        CAMERA.position = Vector3Add(CAMERA.position, up);
        CAMERA.target = Vector3Add(CAMERA.target, up);
    }

    // rotate
    if (is_rotating) {
        CameraYaw(&CAMERA, -ROT_SPEED * mouse_delta.x, true);
        CameraPitch(&CAMERA, ROT_SPEED * mouse_delta.y, true, true, false);
    }

    // zoom
    CameraMoveToTarget(&CAMERA, -GetMouseWheelMove() * ZOOM_SPEED);
}

static void update_first_person_mode() {
    auto view = globals::registry.view<component::Player>();
    if (view.empty()) return;
    auto player = view.front();

    Vector3 position = transform::get_world_position(player);
    position.y += gameplay_config::PLAYER_HEIGHT;

    Vector3 forward = transform::get_forward(player);
    Vector3 target = Vector3Add(position, forward);

    CAMERA.position = position;
    CAMERA.target = target;
}

void update() {
    switch (globals::GAME_STATE) {
        case globals::GameState::EDITOR: update_editor_mode(); break;
        case globals::GameState::PLAY: update_first_person_mode(); break;
    }
}

}  // namespace soft_tissues::system::camera
