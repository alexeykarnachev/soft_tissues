#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "component/light.hpp"
#include "component/transform.hpp"
#include "controller.hpp"
#include "editor.hpp"
#include "globals.hpp"
#include "prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "resources.hpp"
#include "world.hpp"

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");
    ToggleFullscreen();

    // InitWindow(2560 / 2, 1440 / 2, "Soft Tissues");

    DisableCursor();
    // SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;

static void load() {
    load_window();
    resources::load();
    editor::load();
    world::load();

    prefabs::spawn_player(world::get_center());
    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, globals::PLAYER_HEIGHT, 16));

    // light
    {
        Vector3 position = {10, 10, 10};
        light::Type type = light::Type::POINT;
        Color color = BLUE;
        float intensity = 5.0;
        Vector3 attenuation = {1.0, 0.1, 0.01};
        light::Params params = {.point = {.attenuation = attenuation}};
        prefabs::spawn_light(position, type, color, intensity, params);

        position = {20, 20, 10};
        type = light::Type::POINT;
        color = RED;
        intensity = 5.0;
        attenuation = {1.0, 0.1, 0.01};
        params = {.point = {.attenuation = attenuation}};
        prefabs::spawn_light(position, type, color, intensity, params);

        position = {15, 15, 10};
        type = light::Type::POINT;
        color = GREEN;
        intensity = 5.0;
        attenuation = {1.0, 0.1, 0.01};
        params = {.point = {.attenuation = attenuation}};
        prefabs::spawn_light(position, type, color, intensity, params);

        position = Vector3Zero();
        type = light::Type::SPOT;
        color = {255, 255, 220, 255};
        intensity = 50.0;
        attenuation = {1.0, 1.2, 0.2};
        Vector3 direction = {0.0, 0.0, -1.0};
        float inner_cutoff = 0.95;
        float outer_cutoff = 0.80;

        params
            = {.spot = {
                   .attenuation = attenuation,
                   .direction = direction,
                   .inner_cutoff = inner_cutoff,
                   .outer_cutoff = outer_cutoff,
               }};
        prefabs::spawn_light(position, type, color, intensity, params);
    }
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

    // --------------------------
    // TODO: remove this
    auto player = globals::registry.view<component::Player>().front();
    auto player_tr = globals::registry.get<component::Transform>(player);
    int light_idx = 0;
    for (auto entity : globals::registry.view<light::Light>()) {
        auto &light_tr = globals::registry.get<component::Transform>(entity);
        light_tr.position = player_tr.position;
        light_tr.position.y += globals::PLAYER_HEIGHT;

        auto &light = globals::registry.get<component::Light>(entity);
        light.params.spot.direction = player_tr.get_forward();
    }
    // --------------------------
}

void draw_cursor() {
    static float radius = 5.0;
    static Color color = WHITE;

    float x = 0.5 * GetScreenWidth();
    float y = 0.5 * GetScreenHeight();
    DrawCircle(x, y, radius, color);
}

void draw_player() {
    auto player = globals::registry.view<component::Player>().front();
    auto tr = globals::registry.get<component::Transform>(player);
    Matrix matrix = MatrixTranslate(tr.position.x, tr.position.y, tr.position.z);

    Model model = PLAYER_MODEL;
    model.transform = MatrixMultiply(model.transform, matrix);
    DrawModel(model, Vector3Zero(), 1.0, WHITE);
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    // -------------------------------------------------------------------
    // draw world space
    BeginMode3D(camera::CAMERA);
    {
        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
        }

        world::draw_tiles();
    }
    EndMode3D();

    // -------------------------------------------------------------------
    // draw screen space
    {
        if (globals::GAME_STATE == globals::GameState::PLAY) {
            draw_cursor();
        } else if (globals::GAME_STATE == globals::GameState::EDITOR) {
            editor::update_and_draw();
        }
        DrawFPS(0, 0);
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
