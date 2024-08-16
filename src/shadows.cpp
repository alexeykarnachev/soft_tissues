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

ShadowMap::ShadowMap() = default;

void ShadowMap::load() {
    this->fbo = rlLoadFramebuffer();
    this->texture = rlLoadTextureDepth(
        globals::SHADOW_MAP_SIZE, globals::SHADOW_MAP_SIZE, false
    );
    rlFramebufferAttach(
        this->fbo, this->texture, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0
    );

    if (!rlFramebufferComplete(this->fbo)) {
        throw std::runtime_error("Failed to create shadow map fbo");
    }
}

void ShadowMap::unload() {
    rlUnloadFramebuffer(this->fbo);
    rlUnloadTexture(this->texture);
}

static std::array<ShadowMap, globals::MAX_N_SHADOW_MAPS> SHADOW_MAPS;

void load() {
    for (auto &shadow_map : SHADOW_MAPS) {
        shadow_map.load();
    }
}

void unload() {
    for (auto &shadow_map : SHADOW_MAPS) {
        shadow_map.unload();
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

        switch (light->type) {
            case light::Type::SPOT: {
                auto map = SHADOW_MAPS[map_idx++];

                rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, map.fbo);
                rlClearScreenBuffers();

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
                light->view_proj = MatrixMultiply(v, p);
                light->shadow_map = map;

                // draw scene
                // TODO: Maybe factor out into draw_scene()
                world::draw_tiles();
                world::draw_meshes();

                EndMode3D();
                rlBindFramebuffer(RL_DRAW_FRAMEBUFFER, 0);
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
