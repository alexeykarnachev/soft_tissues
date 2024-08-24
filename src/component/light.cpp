#include "light.hpp"

#include "../globals.hpp"
#include "../resources.hpp"
#include "../utils.hpp"
#include "../world.hpp"
#include "component.hpp"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include <stdexcept>
#include <string>

namespace soft_tissues::light {

Light::Light(
    entt::entity entity, LightType light_type, Color color, float intensity, Params params
)
    : entity(entity)
    , is_on(true)
    , casts_shadows(false)
    , light_type(light_type)
    , shadow_type(ShadowType::DYNAMIC)
    , color(color)
    , intensity(intensity)
    , shadow_map(NULL)
    , params(params) {}

void Light::draw_shadow_map() {
    if (!this->casts_shadows || !this->is_on) return;

    // -------------------------------------------------------------------
    // decide if the shadow map needs to be updated
    if (this->casts_shadows) {

        if (this->shadow_type == ShadowType::DYNAMIC) {
            this->needs_update = true;
        }

    } else {
        this->needs_update = false;

        // update is not needed, but shadow map is assigned, so we return it back
        // to the resources manager
        if (this->shadow_map != NULL) {
            resources::free_shadow_map(this->shadow_map);
            this->shadow_map = NULL;
        }
    }

    if (!this->needs_update) {
        return;
    }

    // -------------------------------------------------------------------
    // assign shadow map if not assigned yet
    if (this->shadow_map == NULL) {
        this->shadow_map = resources::get_shadow_map();

        if (this->shadow_map == NULL) {
            return;
        }
    }

    // -------------------------------------------------------------------
    // draw shadow map
    auto tr = globals::registry.get<component::Transform>(this->entity);
    Vector3 position = tr.get_position();
    Vector3 direction = tr.get_forward();

    switch (this->light_type) {
        case LightType::SPOT: {
            BeginTextureMode(*this->shadow_map);
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
            this->vp_mat = MatrixMultiply(v, p);

            // draw scene
            // TODO: Maybe refactor into push_shadow_map_pass / pop_ ...
            // or maybe just use RenderTexture.depth
            bool is_shadow_map_pass = globals::IS_SHADOW_MAP_PASS;
            globals::IS_SHADOW_MAP_PASS = true;

            // TODO: Maybe factor out into draw_scene()
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

            this->casts_shadows = false;
        } break;
    }

    // render static shadow map only once
    if (this->shadow_type == ShadowType::STATIC) {
        needs_update = false;
    }
}

static int get_uniform_loc(Shader shader, int idx, std::string param_name) {
    std::string name = "u_lights[" + std::to_string(idx) + "]";
    name = name + "." + param_name;

    return utils::get_uniform_loc(shader, name);
}

void Light::set_shader_uniform(Shader shader, int idx) {
    if (!this->is_on) {
        throw std::runtime_error("Can't set light shader uniform, is_on = false");
    }

    auto tr = globals::registry.get<component::Transform>(this->entity);
    Vector3 direction = tr.get_forward();
    Vector4 color = ColorNormalize(this->color);

    // -------------------------------------------------------------------
    // shadow map
    if (this->shadow_map != NULL) {
        // NOTE: this "10" is an arbitrary slot number,
        // maybe I should factor out it somehow!
        int slot = 10 + idx;

        rlActiveTextureSlot(slot);
        rlEnableTexture(this->shadow_map->texture.id);
        SetShaderValue(
            shader,
            GetShaderLocation(shader, TextFormat("u_shadow_maps[%d]", idx)),
            &slot,
            SHADER_UNIFORM_INT
        );
    }

    // -------------------------------------------------------------------
    // common params
    int position_loc = get_uniform_loc(shader, idx, "position");
    int type_loc = get_uniform_loc(shader, idx, "type");
    int color_loc = get_uniform_loc(shader, idx, "color");
    int intensity_loc = get_uniform_loc(shader, idx, "intensity");
    int casts_shadows_loc = get_uniform_loc(shader, idx, "casts_shadows");
    int vp_mat_loc = get_uniform_loc(shader, idx, "vp_mat");

    int casts_shadows = (int)this->casts_shadows;

    Vector3 position = tr.get_position();
    SetShaderValue(shader, position_loc, &position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, type_loc, &this->light_type, SHADER_UNIFORM_INT);
    SetShaderValue(shader, color_loc, &color, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, intensity_loc, &this->intensity, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, casts_shadows_loc, &casts_shadows, SHADER_UNIFORM_INT);
    SetShaderValueMatrix(shader, vp_mat_loc, this->vp_mat);

    // -------------------------------------------------------------------
    // type params
    switch (this->light_type) {
        case LightType::POINT: {
            int attenuation_loc = get_uniform_loc(shader, idx, "attenuation");

            Vector3 attenuation = this->params.point.attenuation;

            SetShaderValue(shader, attenuation_loc, &attenuation, SHADER_UNIFORM_VEC3);
        } break;
        case LightType::DIRECTIONAL: {
            int direction_loc = get_uniform_loc(shader, idx, "direction");

            SetShaderValue(shader, direction_loc, &direction, SHADER_UNIFORM_VEC3);
        } break;
        case LightType::SPOT: {
            int attenuation_loc = get_uniform_loc(shader, idx, "attenuation");
            int direction_loc = get_uniform_loc(shader, idx, "direction");
            int inner_cutoff_loc = get_uniform_loc(shader, idx, "inner_cutoff");
            int outer_cutoff_loc = get_uniform_loc(shader, idx, "outer_cutoff");

            Vector3 attenuation = this->params.spot.attenuation;
            float inner_cutoff = this->params.spot.inner_cutoff;
            float outer_cutoff = this->params.spot.outer_cutoff;

            SetShaderValue(shader, attenuation_loc, &attenuation, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, direction_loc, &direction, SHADER_UNIFORM_VEC3);
            SetShaderValue(shader, inner_cutoff_loc, &inner_cutoff, SHADER_UNIFORM_FLOAT);
            SetShaderValue(shader, outer_cutoff_loc, &outer_cutoff, SHADER_UNIFORM_FLOAT);
        } break;
        default: break;
    }
}

std::string get_light_type_name(LightType type) {
    switch (type) {
        case LightType::POINT: return "POINT";
        case LightType::DIRECTIONAL: return "DIRECTIONAL";
        case LightType::SPOT: return "SPOT";
        case LightType::AMBIENT: return "AMBIENT";
        default: throw std::runtime_error("Failed to get light type name");
    }
}

std::string get_shadow_type_name(ShadowType type) {
    switch (type) {
        case ShadowType::STATIC: return "STATIC";
        case ShadowType::DYNAMIC: return "DYNAMIC";
        default: throw std::runtime_error("Failed to get shadow type name");
    }
}

}  // namespace soft_tissues::light
