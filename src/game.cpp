#include "game.hpp"

#include "system/camera.hpp"
#include "component/component.hpp"
#include "system/controller.hpp"
#include "editor/editor.hpp"
#include "globals.hpp"
#include "core/prefabs.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "core/resources.hpp"
#include "system/lighting.hpp"
#include "system/render.hpp"
#include "system/scene.hpp"
#include "core/world.hpp"

namespace soft_tissues::game {

static void load_window() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);

    InitWindow(1920, 1080, "Soft Tissues");

    DisableCursor();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
}

static void update_game_state() {
    if (IsKeyPressed(KEY_F1)) {
        if (globals::GAME_STATE == globals::GameState::PLAY) {
            globals::GAME_STATE = globals::GameState::EDITOR;
            EnableCursor();
        } else if (globals::GAME_STATE == globals::GameState::EDITOR) {
            globals::GAME_STATE = globals::GameState::PLAY;
            DisableCursor();
        }
    }
}

static bool update() {
    bool should_close = globals::update();
    should_close = should_close || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_F4));
    update_game_state();

    if (globals::GAME_STATE == globals::GameState::PLAY) {
        system::controller::update();
    }

    system::camera::update();

    return should_close;
}

static void draw_cursor() {
    static const float RADIUS = 5.0;
    static const Color COLOR = WHITE;

    float x = 0.5 * GetScreenWidth();
    float y = 0.5 * GetScreenHeight();
    DrawCircle(x, y, RADIUS, COLOR);
}

static void draw() {
    const auto &render_state = globals::RENDER_STATE;
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

    BeginMode3D(system::camera::CAMERA);
    {
        if (globals::GAME_STATE == globals::GameState::EDITOR) {
            system::scene::draw_player();
            system::scene::draw_light_shells();
            system::scene::draw_grid();
        }

        if (render_state.is_light_enabled) {
            system::lighting::set_light_uniforms(pbr_shader);
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

static void on_shadow_data_destroyed(entt::registry &reg, entt::entity entity) {
    auto *sd = reg.try_get<component::ShadowData>(entity);
    if (sd != nullptr && sd->shadow_map != nullptr) {
        resources::free_shadow_map(sd->shadow_map);
        sd->shadow_map = nullptr;
    }
}

void run() {
    // load engine
    load_window();
    resources::load();
    editor::load();

    // register cleanup hooks
    globals::registry.on_destroy<component::ShadowData>().connect<&on_shadow_data_destroyed>();

    // load initial scene
    globals::registry.clear();
    world::reset();
    prefabs::spawn_player(world::ORIGIN);

    // main loop
    while (true) {
        if (update()) break;
        draw();
    }

    // unload
    globals::registry.clear();
    editor::unload();
    resources::unload();
    CloseWindow();
}

}  // namespace soft_tissues::game
