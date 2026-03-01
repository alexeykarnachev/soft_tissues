#include "lighting.hpp"

#include "../component/component.hpp"
#include "../globals.hpp"
#include "../pbr.hpp"
#include "../resources.hpp"
#include "../world.hpp"
#include "render.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include <stdexcept>

namespace soft_tissues::system::lighting {

void draw_shadow_maps() {
    for (auto entity : globals::registry.view<component::Light>()) {
        auto &light = globals::registry.get<component::Light>(entity);

        // if shadows were turned off, free the shadow map back to the pool
        if (!light.casts_shadows && light.shadow_map != nullptr) {
            resources::free_shadow_map(light.shadow_map);
            light.shadow_map = nullptr;
        }

        if (!light.casts_shadows || !light.is_on) continue;

        if (light.shadow_type == light::ShadowType::DYNAMIC) {
            light.needs_update = true;
        }

        if (!light.needs_update) continue;

        // assign shadow map if not assigned yet
        if (light.shadow_map == nullptr) {
            light.shadow_map = resources::get_shadow_map();

            if (light.shadow_map == nullptr) {
                continue;
            }
        }

        // draw shadow map
        auto tr = globals::registry.get<component::Transform>(entity);
        Vector3 position = tr.get_position();
        Vector3 direction = tr.get_forward();

        switch (light.light_type) {
            case light::LightType::SPOT: {
                BeginTextureMode(*light.shadow_map);
                ClearBackground(YELLOW);

                Camera3D camera = {0};
                camera.position = position;
                camera.target = Vector3Add(position, direction);
                // TODO: Implement and use tr.get_up or even tr.get_camera
                camera.up = {0.0, 1.0, 0.0};
                camera.fovy = 90.0;
                camera.projection = CAMERA_PERSPECTIVE;

                BeginMode3D(camera);

                Matrix v = rlGetMatrixModelview();
                Matrix p = rlGetMatrixProjection();
                light.vp_mat = MatrixMultiply(v, p);

                // draw scene
                bool is_shadow_map_pass = globals::IS_SHADOW_MAP_PASS;
                globals::IS_SHADOW_MAP_PASS = true;

                render::begin_frame(resources::get_pbr_shader());
                world::draw_tiles();
                world::draw_meshes();

                globals::IS_SHADOW_MAP_PASS = is_shadow_map_pass;

                EndMode3D();
                EndTextureMode();
            } break;
            default: {
                TraceLog(
                    LOG_WARNING, "Shadow mapping for this type of light is not implemented"
                );

                light.casts_shadows = false;
            } break;
        }

        // render static shadow map only once
        if (light.shadow_type == light::ShadowType::STATIC) {
            light.needs_update = false;
        }
    }
}

void set_light_uniforms(pbr::PBRShader &pbr_shader) {
    int light_idx = 0;

    for (auto entity : globals::registry.view<component::Light>()) {
        if (light_idx >= globals::MAX_N_LIGHTS) break;

        auto &light = globals::registry.get<component::Light>(entity);
        if (!light.is_on) continue;

        // set_shader_uniform logic
        Shader shader = pbr_shader.get_shader();
        const auto &locs = pbr_shader.get_light_locs(light_idx);

        auto tr = globals::registry.get<component::Transform>(entity);
        Vector3 direction = tr.get_forward();
        Vector4 color = ColorNormalize(light.color);

        // shadow map
        if (light.shadow_map != nullptr) {
            // NOTE: this "10" is an arbitrary slot number,
            // maybe I should factor out it somehow!
            int slot = 10 + light_idx;

            rlActiveTextureSlot(slot);
            rlEnableTexture(light.shadow_map->texture.id);
            SetShaderValue(shader, locs.shadow_map, &slot, SHADER_UNIFORM_INT);
        }

        // common params
        int casts_shadows = (int)light.casts_shadows;

        Vector3 position = tr.get_position();
        SetShaderValue(shader, locs.position, &position, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.type, &light.light_type, SHADER_UNIFORM_INT);
        SetShaderValue(shader, locs.color, &color, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.intensity, &light.intensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, locs.casts_shadows, &casts_shadows, SHADER_UNIFORM_INT);
        SetShaderValueMatrix(shader, locs.vp_mat, light.vp_mat);

        // type params
        switch (light.light_type) {
            case light::LightType::POINT: {
                Vector3 attenuation = light.params.point.attenuation;
                SetShaderValue(shader, locs.attenuation, &attenuation, SHADER_UNIFORM_VEC3);
            } break;
            case light::LightType::DIRECTIONAL: {
                SetShaderValue(shader, locs.direction, &direction, SHADER_UNIFORM_VEC3);
            } break;
            case light::LightType::SPOT: {
                Vector3 attenuation = light.params.spot.attenuation;
                float inner_cutoff = light.params.spot.inner_cutoff;
                float outer_cutoff = light.params.spot.outer_cutoff;

                SetShaderValue(shader, locs.attenuation, &attenuation, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, locs.direction, &direction, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, locs.inner_cutoff, &inner_cutoff, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, locs.outer_cutoff, &outer_cutoff, SHADER_UNIFORM_FLOAT);
            } break;
            default: break;
        }

        light_idx++;
    }

    pbr_shader.set_n_lights(light_idx);
}

}  // namespace soft_tissues::system::lighting
