#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "component/light.hpp"
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

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    // InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");
    // ToggleFullscreen();

    InitWindow(2560 / 1.5, 1440 / 1.5, "Soft Tissues");

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;

static void load() {
    load_window();
    resources::load();
    editor::load();
    world::load();

    prefabs::spawn_player(world::ORIGIN);
    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, globals::PLAYER_HEIGHT, 16));

    // light
    {
        // Vector3 position = {10, 10, 10};
        // light::Type type = light::Type::POINT;
        // Color color = BLUE;
        // float intensity = 5.0;
        // Vector3 attenuation = {1.0, 0.1, 0.01};
        // light::Params params = {.point = {.attenuation = attenuation}};
        // prefabs::spawn_light(position, type, color, intensity, params);

        Vector3 position = Vector3Zero();
        auto type = light::Type::SPOT;
        Color color = {255, 255, 220, 255};
        auto intensity = 50.0;
        Vector3 attenuation = {1.0, 1.2, 0.2};
        Vector3 direction = {0.0, 0.0, -1.0};
        float inner_cutoff = 0.95;
        float outer_cutoff = 0.80;

        light::Params params
            = {.spot = {
                   .attenuation = attenuation,
                   .direction = direction,
                   .inner_cutoff = inner_cutoff,
                   .outer_cutoff = outer_cutoff,
               }};
        prefabs::spawn_light(position, type, color, intensity, params);

        prefabs::spawn_ambient_light(WHITE, 0.1);
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
    // TODO: move this from here
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

    Matrix t = MatrixTranslate(tr.position.x, tr.position.y, tr.position.z);
    Matrix r = QuaternionToMatrix(tr.get_quaternion());
    Matrix transform = MatrixMultiply(r, t);

    Model model = PLAYER_MODEL;
    model.transform = MatrixMultiply(model.transform, transform);
    DrawModel(model, Vector3Zero(), 1.0, {220, 95, 30, 255});
}

static void draw() {
    BeginDrawing();
    rlEnableDepthTest();
    ClearBackground(BLANK);

    BeginMode3D(camera::CAMERA);
    {
        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
            world::draw_grid();
        }

        world::draw_tiles();
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
    load();

    while (!globals::WINDOW_SHOULD_CLOSE) {
        update();
        draw();
    }

    unload();
}

}  // namespace soft_tissues::game
