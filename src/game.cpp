#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "controller.hpp"
#include "editor.hpp"
#include "globals.hpp"
#include "prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "resources.hpp"

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
    resources::load();
    editor::load();

    prefabs::spawn_player(Vector3Zero());
}

static void unload() {
    editor::unload();
    resources::unload();
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

    if (globals::GAME_STATE == globals::GameState::PLAY) {
        movement::update();
    }

    camera::update();
}

void draw_cursor() {
    static float radius = 5.0;
    static Color color = WHITE;

    float x = 0.5 * GetScreenWidth();
    float y = 0.5 * GetScreenHeight();
    DrawCircle(x, y, radius, color);
}

void draw_player() {
    static float radius = 0.25;
    static int n_slices = 16;
    static Color color = {0, 255, 0, 255};

    auto player = globals::registry.view<component::Player>().front();
    auto tr = globals::registry.get<component::Transform>(player);

    Vector3 start_pos = tr.position;
    Vector3 end_pos = start_pos;
    end_pos.y += globals::PLAYER_HEIGHT;

    DrawCylinder(tr.position, radius, radius, globals::PLAYER_HEIGHT, n_slices, color);
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    // -------------------------------------------------------------------
    // draw world space
    BeginMode3D(camera::CAMERA);
    {
        DrawGrid(10.0, 1.0);

        // if (globals::GAME_STATE == globals::GameState::EDITOR) {
        //     draw_player();
        // }

        DrawModel(resources::PLANE_MODEL, Vector3Zero(), 1.0, WHITE);
    }
    EndMode3D();

    // -------------------------------------------------------------------
    // draw screen space
    {
        // if (globals::GAME_STATE == globals::GameState::PLAY) {
        //     draw_cursor();
        // } else if (globals::GAME_STATE == globals::GameState::EDITOR) {
        //     editor::update_and_draw();
        // }
        // DrawFPS(0, 0);
    }

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
