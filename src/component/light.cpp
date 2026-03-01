#include "light.hpp"

#include "../serializers.hpp"
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
    , shadow_map(nullptr)
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
        case LightType::POINT:
            json["params"] = {
                {"attenuation", this->params.point.attenuation},
            };
            break;
        case LightType::SPOT:
            json["params"] = {
                {"attenuation", this->params.spot.attenuation},
                {"inner_cutoff", this->params.spot.inner_cutoff},
                {"outer_cutoff", this->params.spot.outer_cutoff}
            };
            break;
        default: json["params"] = {}; break;
    }

    return json;
}

Light Light::from_json(entt::entity entity, const nlohmann::json &json_data) {
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
    Params params = {0};
    switch (light_type) {
        case LightType::POINT:
            params.point.attenuation = json_data["params"]["attenuation"];
            break;
        case LightType::SPOT:
            params.spot.attenuation = json_data["params"]["attenuation"];
            params.spot.inner_cutoff = json_data["params"]["inner_cutoff"];
            params.spot.outer_cutoff = json_data["params"]["outer_cutoff"];
            break;
        default: break;
    }

    Light light(entity, light_type, color, intensity, params);

    light.is_on = is_on;
    light.casts_shadows = casts_shadows;
    light.shadow_type = shadow_type;

    return light;
}

}  // namespace soft_tissues::light
