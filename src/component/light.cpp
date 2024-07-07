#include "light.hpp"

#include "../drawing.hpp"
#include "../globals.hpp"
#include "component.hpp"
#include "raylib/raylib.h"
#include <string>

namespace soft_tissues::light {

Light::Light(entt::entity entity, Type type, Color color, float intensity, Params params)
    : entity(entity)
    , type(type)
    , color(color)
    , intensity(intensity)
    , params(params) {}

static int get_uniform_loc(Shader shader, int idx, std::string param_name) {
    std::string name = "u_lights[" + std::to_string(idx) + "]";
    name = name + "." + param_name;

    return drawing::get_uniform_loc(shader, name);
}

void Light::set_shader_uniform(Shader shader, int idx) {
    auto tr = globals::registry.get<component::Transform>(this->entity);
    Vector4 color = ColorNormalize(this->color);

    // -------------------------------------------------------------------
    // common params
    int position_loc = get_uniform_loc(shader, idx, "position");
    int type_loc = get_uniform_loc(shader, idx, "type");
    int color_loc = get_uniform_loc(shader, idx, "color");
    int intensity_loc = get_uniform_loc(shader, idx, "intensity");

    SetShaderValue(shader, position_loc, &tr.position, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, type_loc, &this->type, SHADER_UNIFORM_INT);
    SetShaderValue(shader, color_loc, &color, SHADER_UNIFORM_VEC3);
    SetShaderValue(shader, intensity_loc, &this->intensity, SHADER_UNIFORM_FLOAT);

    // -------------------------------------------------------------------
    // type params
    switch (this->type) {
        case Type::POINT: {
            int attenuation_loc = get_uniform_loc(shader, idx, "attenuation");
            Vector3 attenuation = this->params.point.attenuation;

            SetShaderValue(shader, attenuation_loc, &attenuation, SHADER_UNIFORM_VEC3);
        } break;
        case Type::DIRECTIONAL: {
            int direction_loc = get_uniform_loc(shader, idx, "direction");
            Vector3 direction = this->params.directional.direction;

            SetShaderValue(shader, direction_loc, &direction, SHADER_UNIFORM_VEC3);
        } break;
    }
}

}  // namespace soft_tissues::light
