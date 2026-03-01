#include "lighting.hpp"

#include "../component/component.hpp"
#include "../globals.hpp"
#include "../pbr.hpp"
#include "../resources.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include <unordered_map>

namespace soft_tissues::system::lighting {

static std::unordered_map<entt::entity, ShadowData> SHADOW_DATA;

ShadowData *get_shadow_data(entt::entity entity) {
    auto it = SHADOW_DATA.find(entity);
    if (it == SHADOW_DATA.end()) return nullptr;
    return &it->second;
}

void cleanup_shadow_data(entt::entity entity) {
    auto it = SHADOW_DATA.find(entity);
    if (it == SHADOW_DATA.end()) return;

    if (it->second.shadow_map != nullptr) {
        resources::free_shadow_map(it->second.shadow_map);
    }

    SHADOW_DATA.erase(it);
}

void cleanup_all_shadow_data() {
    for (auto &[entity, sd] : SHADOW_DATA) {
        if (sd.shadow_map != nullptr) {
            resources::free_shadow_map(sd.shadow_map);
        }
    }

    SHADOW_DATA.clear();
}

std::vector<ShadowPassJob> prepare_shadow_passes() {
    std::vector<ShadowPassJob> jobs;

    for (auto entity : globals::registry.view<component::Light>()) {
        auto &light = globals::registry.get<component::Light>(entity);
        auto &sd = SHADOW_DATA[entity];

        // if shadows were turned off, free the shadow map back to the pool
        if (!light.casts_shadows && sd.shadow_map != nullptr) {
            resources::free_shadow_map(sd.shadow_map);
            sd.shadow_map = nullptr;
        }

        if (!light.casts_shadows || !light.is_on) continue;

        if (light.shadow_type == light::ShadowType::DYNAMIC) {
            sd.needs_update = true;
        }

        if (!sd.needs_update) continue;

        // assign shadow map if not assigned yet
        if (sd.shadow_map == nullptr) {
            sd.shadow_map = resources::get_shadow_map();

            if (sd.shadow_map == nullptr) {
                continue;
            }
        }

        // build camera for this light
        auto tr = globals::registry.get<component::Transform>(entity);
        Vector3 position = tr.get_position();
        Vector3 direction = tr.get_forward();

        switch (light.light_type) {
            case light::LightType::SPOT: {
                Camera3D camera = {0};
                camera.position = position;
                camera.target = Vector3Add(position, direction);
                camera.up = {0.0, 1.0, 0.0};
                camera.fovy = 90.0;
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
    auto &sd = SHADOW_DATA[entity];
    sd.vp_mat = vp_mat;

    auto &light = globals::registry.get<component::Light>(entity);
    if (light.shadow_type == light::ShadowType::STATIC) {
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

        auto tr = globals::registry.get<component::Transform>(entity);
        Vector3 direction = tr.get_forward();
        Vector4 color = ColorNormalize(light.color);

        // shadow map
        auto *sd = get_shadow_data(entity);
        if (sd != nullptr && sd->shadow_map != nullptr) {
            int slot = 10 + light_idx;

            rlActiveTextureSlot(slot);
            rlEnableTexture(sd->shadow_map->texture.id);
            SetShaderValue(shader, locs.shadow_map, &slot, SHADER_UNIFORM_INT);
        }

        // common params
        int casts_shadows = (int)light.casts_shadows;
        Matrix vp_mat = (sd != nullptr) ? sd->vp_mat : MatrixIdentity();

        Vector3 position = tr.get_position();
        SetShaderValue(shader, locs.position, &position, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.type, &light.light_type, SHADER_UNIFORM_INT);
        SetShaderValue(shader, locs.color, &color, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, locs.intensity, &light.intensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, locs.casts_shadows, &casts_shadows, SHADER_UNIFORM_INT);
        SetShaderValueMatrix(shader, locs.vp_mat, vp_mat);

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
