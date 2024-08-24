#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "component/transform.hpp"
#include "controller.hpp"
#include "editor/editor.hpp"
#include "globals.hpp"
#include "prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "resources.hpp"
#include "world.hpp"
#include <cstdio>

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    // InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");
    // ToggleFullscreen();

    InitWindow(1920, 1080, "Soft Tissues");

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;

void load() {
    reset();

    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, globals::PLAYER_HEIGHT, 16));
}

void reset() {
    for (const auto entity : globals::registry.view<transform::Transform>()) {
        globals::registry.destroy(entity);
    }

    prefabs::spawn_player(world::ORIGIN);

    world::load();

    TraceLog(LOG_INFO, "Game has been reset");
}

#if 0
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
#endif

static void update() {
    globals::update();

    if (globals::GAME_STATE == globals::GameState::PLAY) {
        controller::update();
    }

    camera::update();
}

static void draw_cursor() {
    static float radius = 5.0;
    static Color color = WHITE;

    float x = 0.5 * GetScreenWidth();
    float y = 0.5 * GetScreenHeight();
    DrawCircle(x, y, radius, color);
}

static void draw_light_shells() {
    auto view = globals::registry.view<component::Light>();
    for (auto entity : view) {
        auto tr = globals::registry.get<component::Transform>(entity);
        DrawSphere(tr.get_position(), 0.2, WHITE);
    }
}

static void draw_player() {
    auto player = globals::registry.view<component::Player>().front();
    auto tr = globals::registry.get<component::Transform>(player);

    Vector3 position = tr.get_position();
    Matrix t = MatrixTranslate(position.x, position.y, position.z);
    Matrix transform = t;

    Model model = PLAYER_MODEL;
    model.transform = MatrixMultiply(model.transform, transform);
    DrawModel(model, Vector3Zero(), 1.0, {220, 95, 30, 255});
}

static void draw() {
    // -------------------------------------------------------------------
    // shadow maps
    for (auto entity : globals::registry.view<component::Light>()) {
        auto &light = globals::registry.get<component::Light>(entity);
        light.draw_shadow_map();
    }

    // -------------------------------------------------------------------
    // entity picking
    if (globals::GAME_STATE == globals::GameState::EDITOR) {
        editor::update_hovered_entity();
    }

    // -------------------------------------------------------------------
    // main screen
    BeginDrawing();
    rlEnableDepthTest();
    ClearBackground(BLANK);

    // TODO: Maybe factor out BeginMode3D and EndMode3D, such that some
    // global camera (e.g from RENDER_OPTIONS) will be assigned.
    // It means, that the engine should control current camera via
    // RENDER_OPTIONS (but not directly via BeginMode3D(camera::CAMERA)).
    BeginMode3D(camera::CAMERA);
    {
        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
            draw_light_shells();
            world::draw_grid();
        }

        world::draw_tiles();
        world::draw_meshes();
    }
    EndMode3D();

    if (globals::GAME_STATE == globals::GameState::PLAY) {
        draw_cursor();
    }

    if (globals::GAME_STATE == globals::GameState::EDITOR) {
        editor::update_and_draw();
    }

    DrawFPS(0, 0);
    EndDrawing();
}

void run() {
    // load engine
    load_window();
    resources::load();
    editor::load();

    // load initial scene
    load();

    // main loop
    while (!globals::WINDOW_SHOULD_CLOSE) {
        update();
        draw();
    }

    // unload
    editor::unload();
    resources::unload();
    CloseWindow();
}

}  // namespace soft_tissues::game
