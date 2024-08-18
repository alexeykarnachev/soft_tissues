#include "shadows.hpp"

#include "component/component.hpp"
#include "component/light.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "world.hpp"
#include <array>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace soft_tissues::shadows {

static std::array<RenderTexture2D, globals::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

void load() {
    for (auto &map : SHADOW_MAPS) {
        map = LoadRenderTexture(globals::SHADOW_MAP_SIZE, globals::SHADOW_MAP_SIZE);
    }
}

void unload() {
    for (auto &map : SHADOW_MAPS) {
        UnloadRenderTexture(map);
    }
}

void draw() {
    static std::vector<component::Light *> lights;
    lights.clear();

    // collect lights-shadow casters
    for (auto entity : globals::registry.view<component::Light>()) {
        if (lights.size() == SHADOW_MAPS.size()) break;

        auto &light = globals::registry.get<component::Light>(entity);
        if (!light.is_enabled || !light.casts_shadows) continue;

        // TODO: Support shadow maps not only for SPOT light.
        if (light.type != light::Type::SPOT) continue;

        lights.push_back(&light);
    }

    // draw shadows
    int map_idx = 0;
    for (auto light : lights) {
        auto tr = globals::registry.get<component::Transform>(light->entity);
        Vector3 position = tr.get_position();
        Vector3 direction = tr.get_forward();

        // TODO: Refactor shadow maps management (e.g 6 maps for point light, etc)
        auto map = SHADOW_MAPS[map_idx];
        map_idx += 6;

        switch (light->type) {
            case light::Type::SPOT: {
                BeginTextureMode(map);
                ClearBackground(YELLOW);

                Camera3D camera = {0};
                camera.position = position;
                camera.target = Vector3Add(position, direction);
                // TODO: Implement and use tr.get_up or even tr.get_camera
                camera.up = {0.0, 1.0, 0.0};
                camera.fovy = 90.0;
                camera.projection = CAMERA_PERSPECTIVE;

                BeginMode3D(camera);

                // update runtime values
                Matrix v = rlGetMatrixModelview();
                Matrix p = rlGetMatrixProjection();
                light->vp_mat = MatrixMultiply(v, p);
                light->shadow_map = map;

                // draw scene
                // TODO: Maybe refactor into push_shadow_map_pass / pop_ ...
                bool is_shadow_map_pass = globals::RENDER_OPTIONS.is_shadow_map_pass;
                globals::RENDER_OPTIONS.is_shadow_map_pass = true;

                // TODO: Maybe factor out into draw_scene()
                world::draw_tiles();
                world::draw_meshes();

                globals::RENDER_OPTIONS.is_shadow_map_pass = is_shadow_map_pass;

                EndMode3D();
                EndTextureMode();
            } break;
            default: {
                throw std::runtime_error(
                    "Shadow mapping for this type of light is not implemented"
                );
            } break;
        }
    }
}

}  // namespace soft_tissues::shadows
