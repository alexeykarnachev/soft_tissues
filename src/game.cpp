#include "game.hpp"

#include "camera.hpp"
#include "cursor.hpp"
#include "editor.hpp"
#include "globals.hpp"
#include "mode.hpp"
#include "movement.hpp"
#include "prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");

    DisableCursor();
    ToggleFullscreen();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static void load() {
    load_window();
    editor::load();

    prefabs::spawn_player(Vector3Zero());
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
    mode::update();
    movement::update();
    camera::update();
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    BeginMode3D(camera::CAMERA);

    DrawGrid(10.0, 1.0);
    EndMode3D();

    editor::update_and_draw();
    cursor::draw();

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
