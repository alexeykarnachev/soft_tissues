#include "game.hpp"

#include "camera.hpp"
#include "editor.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");

    ToggleFullscreen();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static void load() {
    load_window();
    editor::load();
}

static void unload() {
    editor::unload();
    CloseWindow();
}

template <typename T> void update_components() {
    for (auto entity : globals::registry.view<T>()) {
        auto &component = globals::registry.get<T>(entity);
        component.update();
    }
}

template <typename T> void draw_components() {
    for (auto entity : globals::registry.view<T>()) {
        auto &component = globals::registry.get<T>(entity);
        component.draw();
    }
}

static void update() {
    globals::update();
    camera::update();
}

void draw_cube() {
    static Vector3 position = {0.0, 0.0, 0.0};
    static Vector3 size = {1.0, 1.0, 1.0};

    DrawCubeV(position, size, RAYWHITE);
    DrawCubeWiresV(position, size, RED);
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    BeginMode3D(camera::CAMERA);
    DrawGrid(10.0, 1.0);

    if (globals::GAME_STATE == globals::GameState::EDITOR) {
        draw_cube();
    }

    EndMode3D();

    if (globals::GAME_STATE == globals::GameState::EDITOR) {
        editor::update_and_draw();
    }

    DrawFPS(0, 0);
    EndDrawing();
}

void run() {
    load();

    while (!globals::WINDOW_SHOULD_CLOSE) {
        update();
        draw();
    }

    unload();
}

}  // namespace soft_tissues::game
