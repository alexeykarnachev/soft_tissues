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
#include "utils.hpp"

namespace soft_tissues::game {

using namespace utils;

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(globals::SCREEN_WIDTH, globals::SCREEN_HEIGHT, "Soft Tissues");
    ToggleFullscreen();

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;

static void load() {
    load_window();
    resources::load();
    editor::load();

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

void draw_wall() {
    static float height = 3.0;
    static float length = 20.0;
    static float x = 0.0;
    static float y = 0.5 * height;
    static float z = 0.0;
    static float angle = 0.25 * PI * 0.0;
    static float tiling[2] = {length, height};
    static Vector3 ambient_color = {1.0, 1.0, 1.0};
    static float ambient_intensity = 0.01;
    static float displacement_scale = 0.1;

    Material material = resources::PLANE_MODEL.materials[0];
    Shader shader = material.shader;

    // -----------------------------------------------------------------------
    // textures
    int tiling_loc = get_uniform_loc(shader, "u_tiling");
    int displacement_scale_loc = get_uniform_loc(shader, "u_displacement_scale");

    SetShaderValue(shader, tiling_loc, tiling, SHADER_UNIFORM_VEC2);
    SetShaderValue(
        shader, displacement_scale_loc, &displacement_scale, SHADER_UNIFORM_FLOAT
    );

    // -----------------------------------------------------------------------
    // ambient light
    int ambient_color_loc = get_uniform_loc(shader, "u_ambient_color");
    int ambient_intensity_loc = get_uniform_loc(shader, "u_ambient_intensity");

    SetShaderValue(shader, ambient_color_loc, &ambient_color, SHADER_UNIFORM_VEC3);
    SetShaderValue(
        shader, ambient_intensity_loc, &ambient_intensity, SHADER_UNIFORM_FLOAT
    );

    // -----------------------------------------------------------------------
    // lighting
    int light_idx = 0;
    for (auto entity : globals::registry.view<light::Light>()) {
        auto light = globals::registry.get<light::Light>(entity);
        light.set_shader_uniform(shader, light_idx++);
    }

    int camera_pos_loc = get_uniform_loc(shader, "u_camera_pos");
    int n_lights_loc = get_uniform_loc(shader, "u_n_lights");

    SetShaderValue(shader, camera_pos_loc, &camera::CAMERA.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, n_lights_loc, &light_idx, SHADER_UNIFORM_INT);

    // -----------------------------------------------------------------------
    // matrix
    Matrix t = MatrixTranslate(x, y, z);
    Matrix rx = MatrixRotateX(0.5 * PI);
    Matrix ry = MatrixRotateY(angle);
    Matrix r = MatrixMultiply(rx, ry);
    Matrix s = MatrixScale(length, 1.0, height);
    Matrix matrix = MatrixMultiply(s, MatrixMultiply(r, t));

    // -----------------------------------------------------------------------
    // draw
    Model model = resources::PLANE_MODEL;
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
        DrawGrid(20.0, 1.0);

        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
        }

        draw_wall();
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
