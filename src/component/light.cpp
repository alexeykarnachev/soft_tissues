#include "light.hpp"

#include "../serializers.hpp"
#include <stdexcept>
#include <string>

namespace soft_tissues::component {

Light::Light(LightType light_type, Color color, float intensity, LightParams params)
    : is_on(true)
    , casts_shadows(false)
    , light_type(light_type)
    , shadow_type(ShadowType::DYNAMIC)
    , color(color)
    , intensity(intensity)
    , params(params) {}

std::string light_type_to_str(LightType type) {
    switch (type) {
        case LightType::POINT: return "POINT";
        case LightType::DIRECTIONAL: return "DIRECTIONAL";
        case LightType::SPOT: return "SPOT";
        case LightType::AMBIENT: return "AMBIENT";
        default: throw std::runtime_error("Failed to get light type name");
    }
}

std::string shadow_type_to_str(ShadowType type) {
    switch (type) {
        case ShadowType::STATIC: return "STATIC";
        case ShadowType::DYNAMIC: return "DYNAMIC";
        default: throw std::runtime_error("Failed to get shadow type name");
    }
}

LightType str_to_light_type(std::string str) {
    if (str == "POINT") return LightType::POINT;
    if (str == "DIRECTIONAL") return LightType::DIRECTIONAL;
    if (str == "SPOT") return LightType::SPOT;
    if (str == "AMBIENT") return LightType::AMBIENT;
    throw std::runtime_error("Invalid light type string: " + str);
}

ShadowType str_to_shadow_type(std::string str) {
    if (str == "STATIC") return ShadowType::STATIC;
    if (str == "DYNAMIC") return ShadowType::DYNAMIC;
    throw std::runtime_error("Invalid shadow type string: " + str);
}

nlohmann::json Light::to_json() const {
    nlohmann::json json;

    // -------------------------------------------------------------------
    // common params
    json["light_type"] = light_type_to_str(this->light_type);
    json["shadow_type"] = shadow_type_to_str(this->shadow_type);

    json["is_on"] = this->is_on;
    json["casts_shadows"] = this->casts_shadows;
    json["color"] = this->color;
    json["intensity"] = this->intensity;

    // -------------------------------------------------------------------
    // type params
    switch (this->light_type) {
        case LightType::POINT: {
            auto &p = std::get<PointParams>(this->params);
            json["params"] = {{"attenuation", p.attenuation}};
        } break;
        case LightType::SPOT: {
            auto &p = std::get<SpotParams>(this->params);
            json["params"] = {
                {"attenuation", p.attenuation},
                {"inner_cutoff", p.inner_cutoff},
                {"outer_cutoff", p.outer_cutoff}
            };
        } break;
        default: json["params"] = {}; break;
    }

    return json;
}

Light Light::from_json(const nlohmann::json &json_data) {
    // -------------------------------------------------------------------
    // common params
    LightType light_type = str_to_light_type(json_data["light_type"]);
    ShadowType shadow_type = str_to_shadow_type(json_data["shadow_type"]);

    Color color = json_data["color"];
    float intensity = json_data["intensity"];
    bool is_on = json_data["is_on"];
    bool casts_shadows = json_data["casts_shadows"];

    // -------------------------------------------------------------------
    // type params
    LightParams params = PointParams{};
    switch (light_type) {
        case LightType::POINT: {
            PointParams p;
            p.attenuation = json_data["params"]["attenuation"];
            params = p;
        } break;
        case LightType::SPOT: {
            SpotParams p;
            p.attenuation = json_data["params"]["attenuation"];
            p.inner_cutoff = json_data["params"]["inner_cutoff"];
            p.outer_cutoff = json_data["params"]["outer_cutoff"];
            params = p;
        } break;
        case LightType::DIRECTIONAL:
            params = DirectionalParams{};
            break;
        case LightType::AMBIENT:
            params = AmbientParams{};
            break;
    }

    Light light(light_type, color, intensity, params);

    light.is_on = is_on;
    light.casts_shadows = casts_shadows;
    light.shadow_type = shadow_type;

    return light;
}

}  // namespace soft_tissues::component
