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
#include "tiling.hpp"
#include <vector>

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");
    ToggleFullscreen();

    // InitWindow(2560 / 2, 1440 / 2, "Soft Tissues");

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;
static std::vector<tiling::Tile> TILES;

static void load() {
    load_window();
    resources::load();
    editor::load();

    // clang-format off
    auto flags = 
          tiling::TILE_FLOOR 
        | tiling::TILE_CEIL
        | tiling::TILE_NORTH_WALL 
        | tiling::TILE_SOUTH_WALL 
        | tiling::TILE_WEST_WALL
        | tiling::TILE_EAST_WALL;
    // clang-format on

    TILES.push_back(tiling::Tile(
        0,
        flags,
        tiling::TileMaterials(
            resources::TILED_STONE_MATERIAL,
            resources::BRICK_WALL_MATERIAL,
            resources::TILED_STONE_MATERIAL
        )
    ));

    prefabs::spawn_player({0.0, 0.0, 3.0});
    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, globals::PLAYER_HEIGHT, 16));

    // light
    {
        // Vector3 position = Vector3Zero();
        // light::Type type = light::Type::POINT;
        // Color color = WHITE;
        // float intensity = 5.0;
        // Vector3 attenuation = {1.0, 0.1, 0.01};
        // light::Params params = {.point = {.attenuation = attenuation}};
        // prefabs::spawn_light(position, type, color, intensity, params);

        Vector3 position = Vector3Zero();
        light::Type type = light::Type::SPOT;
        Color color = WHITE;
        float intensity = 10.0;
        Vector3 attenuation = {1.0, 0.1, 0.01};
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

void draw_tiles() {
    for (auto tile : TILES) {
        tile.draw();
    }
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    // -------------------------------------------------------------------
    // draw world space
    BeginMode3D(camera::CAMERA);
    {
        DrawGrid(20.0, 1.0);

        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
        }

        draw_tiles();
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
