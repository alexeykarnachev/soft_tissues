#include "controller.hpp"

#include "../component/component.hpp"
#include "../gameplay_config.hpp"
#include "../globals.hpp"
#include "transform.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::system::controller {

static void update_translation() {
    // -------------------------------------------------------------------
    // moving
    Vector3 dir = Vector3Zero();
    if (IsKeyDown(KEY_W)) dir.z -= 1.0;
    if (IsKeyDown(KEY_S)) dir.z += 1.0;
    if (IsKeyDown(KEY_A)) dir.x -= 1.0;
    if (IsKeyDown(KEY_D)) dir.x += 1.0;

    float length = Vector3Length(dir);
    if (length <= EPSILON) return;

    auto view = globals::registry.view<component::Player>();
    if (view.size() == 0) return;
    auto player = view.front();
    auto &tr = globals::registry.get<component::Transform>(player);

    Vector3 forward = transform::get_forward(player);
    Vector3 right = transform::get_right(player);

    forward.y = 0.0;
    right.y = 0.0;

    dir = Vector3Scale(dir, 1.0 / length);
    forward = Vector3Scale(Vector3Normalize(forward), -dir.z);
    right = Vector3Scale(Vector3Normalize(right), dir.x);
    dir = Vector3Normalize(Vector3Add(forward, right));

    Vector3 step = Vector3Scale(dir, globals::FRAME_DT * gameplay_config::PLAYER_SPEED);

    tr.step(step);
}

static void update_rotation() {
    static bool initialized = false;
    if (!initialized) { initialized = true; return; }

    Vector2 mouse_delta = GetMouseDelta();
    mouse_delta = Vector2Scale(mouse_delta, gameplay_config::PLAYER_CAMERA_SENSITIVITY);

    float yaw_delta = -mouse_delta.x;
    float pitch_delta = -mouse_delta.y;
    if (yaw_delta == 0 && pitch_delta == 0) return;

    auto view = globals::registry.view<component::Player>();
    if (view.size() == 0) return;
    auto player = view.front();
    auto &tr = globals::registry.get<component::Transform>(player);

    float pitch = tr.rotation.x + pitch_delta;
    pitch = Clamp(pitch, -0.5 * PI + 0.025, 0.5 * PI - 0.025);
    float yaw = tr.rotation.y + yaw_delta;

    tr.rotation.x = pitch;
    tr.rotation.y = yaw;
}

static void update_flashlight() {
    auto view = globals::registry.view<component::Flashlight>();
    if (view.size() == 0) return;
    auto flashlight = view.front();
    auto &light = globals::registry.get<component::Light>(flashlight);

    // toggle
    light.is_on ^= IsKeyPressed(KEY_L);
}

void update() {
    update_translation();
    update_rotation();
    update_flashlight();
}

void update_game_state() {
    if (IsKeyPressed(KEY_F1)) {
        if (globals::GAME_STATE == globals::GameState::PLAY) {
            globals::GAME_STATE = globals::GameState::EDITOR;
            EnableCursor();
        } else if (globals::GAME_STATE == globals::GameState::EDITOR) {
            globals::GAME_STATE = globals::GameState::PLAY;
            DisableCursor();
        }
    }
}

}  // namespace soft_tissues::system::controller
