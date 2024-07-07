#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "component/light.hpp"
#include "component/transform.hpp"
#include "controller.hpp"
#include "drawing.hpp"
#include "editor.hpp"
#include "globals.hpp"
#include "prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "resources.hpp"

namespace soft_tissues::game {
using namespace soft_tissues::drawing;

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);

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

    // light
    {
        Vector3 position = {5.0, 10.0, -2.0};
        light::Type type = light::Type::POINT;
        Color color = WHITE;
        float intensity = 18.0;
        Vector3 attenuation = {1.0, 0.03, 0.0003};
        light::Params params = {.point = {.attenuation = attenuation}};
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

void draw_wall() {
    static float height = 3.0;
    static float length = 10.0;
    static float x = 0.0;
    static float y = 0.5 * height;
    static float z = -3.0;
    static float tiling[2] = {length, height};
    static int use_normal_map = 1;
    static Vector3 ambient_color = {1.0, 1.0, 1.0};
    static float ambient_intensity = 0.0025;

    Material material = resources::PLANE_MODEL.materials[0];
    Shader shader = material.shader;

    // -----------------------------------------------------------------------
    // textures
    int tiling_loc = get_uniform_loc(shader, "u_tiling");
    int use_normal_map_loc = get_uniform_loc(shader, "u_use_normal_map");

    SetShaderValue(shader, tiling_loc, tiling, SHADER_UNIFORM_VEC2);
    SetShaderValue(shader, use_normal_map_loc, &use_normal_map, SHADER_UNIFORM_INT);

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
    Matrix r = MatrixRotateX(0.5 * PI);
    Matrix s = MatrixScale(length, height, 1.0);
    Matrix matrix = MatrixMultiply(r, MatrixMultiply(s, t));

    // -----------------------------------------------------------------------
    // draw
    rlPushMatrix();
    rlMultMatrixf(MatrixToFloat(matrix));
    DrawModel(resources::PLANE_MODEL, Vector3Zero(), 1.0, WHITE);
    rlPopMatrix();
}

static void draw() {
    BeginDrawing();
    ClearBackground(BLANK);

    // -------------------------------------------------------------------
    // draw world space
    BeginMode3D(camera::CAMERA);
    {
        DrawGrid(10.0, 1.0);

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
