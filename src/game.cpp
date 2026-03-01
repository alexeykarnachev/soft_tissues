#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "transform.hpp"
#include "controller.hpp"
#include "editor/editor.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "resources.hpp"
#include "system/lighting.hpp"
#include "system/render.hpp"
#include "system/scene.hpp"
#include "world.hpp"

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

    InitWindow(1920, 1080, "Soft Tissues");

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static Model PLAYER_MODEL;

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
    auto view = globals::registry.view<component::Player>();
    if (view.size() == 0) return;
    auto player = view.front();
    auto tr = globals::registry.get<component::Transform>(player);

    Vector3 position = tr.get_position();
    Matrix transform = MatrixTranslate(position.x, position.y, position.z);

    Model model = PLAYER_MODEL;
    model.transform = MatrixMultiply(model.transform, transform);
    DrawModel(model, Vector3Zero(), 1.0, {220, 95, 30, 255});
}

static void draw() {
    // -------------------------------------------------------------------
    // shadow maps
    system::lighting::draw_shadow_maps();

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
            system::scene::draw_grid();
        }

        system::render::begin_frame(resources::get_pbr_shader());
        system::scene::draw_tiles();
        system::scene::draw_meshes();
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

static void on_light_destroyed(entt::registry &reg, entt::entity entity) {
    auto &light = reg.get<component::Light>(entity);
    if (light.shadow_map != nullptr) {
        resources::free_shadow_map(light.shadow_map);
        light.shadow_map = nullptr;
    }
}

void run() {
    // load engine
    load_window();
    resources::load();
    editor::load();

    // register cleanup hooks
    globals::registry.on_destroy<component::Light>().connect<&on_light_destroyed>();

    // load initial scene
    world::reset();
    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, globals::PLAYER_HEIGHT, 16));

    // main loop
    while (!globals::WINDOW_SHOULD_CLOSE) {
        update();
        draw();
    }

    // unload
    UnloadModel(PLAYER_MODEL);
    editor::unload();
    resources::unload();
    CloseWindow();
}

}  // namespace soft_tissues::game
