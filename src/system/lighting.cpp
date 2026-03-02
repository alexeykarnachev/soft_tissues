#include "lighting.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "core/pbr.hpp"
#include "core/resources.hpp"
#include "system/transform.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"

namespace soft_tissues::system::lighting {

std::vector<ShadowPassJob> prepare_shadow_passes() {
    std::vector<ShadowPassJob> jobs;

    for (auto entity : globals::registry.view<component::Light>()) {
        auto &light = globals::registry.get<component::Light>(entity);

        // ensure ShadowData component exists
        if (!globals::registry.all_of<component::ShadowData>(entity)) {
            globals::registry.emplace<component::ShadowData>(entity);
        }
        auto &sd = globals::registry.get<component::ShadowData>(entity);

        // if shadows were turned off, free the shadow map back to the pool
        if (!light.casts_shadows && sd.shadow_map != nullptr) {
            resources::free_shadow_map(sd.shadow_map);
            sd.shadow_map = nullptr;
        }

        if (!light.casts_shadows || !light.is_on) continue;

        if (light.shadow_type == component::ShadowType::DYNAMIC) {
            sd.needs_update = true;
        }

        if (!sd.needs_update) continue;

        // assign shadow map if not assigned yet
        if (sd.shadow_map == nullptr) {
            sd.shadow_map = resources::get_shadow_map();

            if (sd.shadow_map == nullptr) {
                TraceLog(LOG_WARNING, "Shadow map pool exhausted, skipping light");
                continue;
            }
        }

        // build camera for this light
        Vector3 position = transform::get_world_position(entity);
        Vector3 direction = transform::get_forward(entity);

        switch (light.light_type) {
            case component::LightType::SPOT: {
                Camera3D camera = {0};
                camera.position = position;
                camera.target = Vector3Add(position, direction);
                camera.up = {0.0, 1.0, 0.0};
                camera.fovy = render_config::SHADOW_CAMERA_FOV;
                camera.projection = CAMERA_PERSPECTIVE;

                jobs.push_back({entity, camera, sd.shadow_map});
            } break;
            default: {
                TraceLog(
                    LOG_WARNING, "Shadow mapping for this type of light is not implemented"
                );

                light.casts_shadows = false;
            } break;
        }
    }

    return jobs;
}

void finalize_shadow_pass(entt::entity entity, Matrix vp_mat) {
    auto &sd = globals::registry.get<component::ShadowData>(entity);
    sd.vp_mat = vp_mat;

    auto &light = globals::registry.get<component::Light>(entity);
    if (light.shadow_type == component::ShadowType::STATIC) {
        sd.needs_update = false;
    }
}

void set_light_uniforms(pbr::PBRShader &pbr_shader) {
    int light_idx = 0;

    for (auto entity : globals::registry.view<component::Light>()) {
        if (light_idx >= render_config::MAX_N_LIGHTS) break;

        auto &light = globals::registry.get<component::Light>(entity);
        if (!light.is_on) continue;

        Shader shader = pbr_shader.get_shader();
        const auto &locs = pbr_shader.get_light_locs(light_idx);

        Vector3 direction = transform::get_forward(entity);
        Vector4 color = ColorNormalize(light.color);

        // shadow map
        auto *sd = globals::registry.try_get<component::ShadowData>(entity);
        if (sd != nullptr && sd->shadow_map != nullptr) {
            int slot = render_config::SHADOW_MAP_TEXTURE_SLOT_OFFSET + light_idx;

            rlActiveTextureSlot(slot);
            rlEnableTexture(sd->shadow_map->texture.id);
            SetShaderValue(shader, locs.shadow_map, &slot, SHADER_UNIFORM_INT);
        }

        // common params
        int casts_shadows = (int)light.casts_shadows;
        Matrix vp_mat = (sd != nullptr) ? sd->vp_mat : MatrixIdentity();

        Vector3 position = transform::get_world_position(entity);
        SetShaderValue(shader, locs.position, &position, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.type, &light.light_type, SHADER_UNIFORM_INT);
        SetShaderValue(shader, locs.color, &color, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.intensity, &light.intensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, locs.casts_shadows, &casts_shadows, SHADER_UNIFORM_INT);
        SetShaderValueMatrix(shader, locs.vp_mat, vp_mat);

        // type params
        switch (light.light_type) {
            case component::LightType::POINT: {
                auto &p = std::get<component::PointParams>(light.params);
                SetShaderValue(shader, locs.attenuation, &p.attenuation, SHADER_UNIFORM_VEC3);
            } break;
            case component::LightType::DIRECTIONAL: {
                SetShaderValue(shader, locs.direction, &direction, SHADER_UNIFORM_VEC3);
            } break;
            case component::LightType::SPOT: {
                auto &p = std::get<component::SpotParams>(light.params);
                SetShaderValue(shader, locs.attenuation, &p.attenuation, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, locs.direction, &direction, SHADER_UNIFORM_VEC3);
                SetShaderValue(shader, locs.inner_cutoff, &p.inner_cutoff, SHADER_UNIFORM_FLOAT);
                SetShaderValue(shader, locs.outer_cutoff, &p.outer_cutoff, SHADER_UNIFORM_FLOAT);
            } break;
            default: break;
        }

        light_idx++;
    }

    pbr_shader.set_n_lights(light_idx);
}

}  // namespace soft_tissues::system::lighting
