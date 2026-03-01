#include "game.hpp"

#include "camera.hpp"
#include "component/component.hpp"
#include "controller.hpp"
#include "editor/editor.hpp"
#include "gameplay_config.hpp"
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
    auto &render_state = globals::RENDER_STATE;
    auto &pbr_shader = resources::get_pbr_shader();

    // -------------------------------------------------------------------
    // shadow maps
    auto jobs = system::lighting::prepare_shadow_passes();
    for (auto &job : jobs) {
        BeginTextureMode(*job.shadow_map);
        ClearBackground(YELLOW);
        BeginMode3D(job.camera);

        Matrix vp_mat = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());

        RenderState shadow_state = render_state;
        shadow_state.is_shadow_map_pass = true;
        system::render::begin_frame(pbr_shader, shadow_state);
        system::scene::draw_tiles(shadow_state);
        system::scene::draw_meshes(shadow_state);

        EndMode3D();
        EndTextureMode();

        system::lighting::finalize_shadow_pass(job.entity, vp_mat);
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

    BeginMode3D(camera::CAMERA);
    {
        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            draw_player();
            draw_light_shells();
            system::scene::draw_grid();
        }

        system::render::begin_frame(pbr_shader, render_state);
        system::scene::draw_tiles(render_state);
        system::scene::draw_meshes(render_state);
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

static void on_light_destroyed(entt::registry &, entt::entity entity) {
    system::lighting::cleanup_shadow_data(entity);
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
    PLAYER_MODEL = LoadModelFromMesh(GenMeshCylinder(0.25, gameplay_config::PLAYER_HEIGHT, 16));

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
