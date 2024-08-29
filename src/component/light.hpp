#pragma once

#include "entt/entity/fwd.hpp"
#include "nlohmann/json.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::light {

enum class LightType {
    POINT = 0,
    DIRECTIONAL,
    SPOT,
    AMBIENT,
};

constexpr std::array<LightType, 4> LIGHT_TYPES = {
    LightType::POINT,
    LightType::DIRECTIONAL,
    LightType::SPOT,
    LightType::AMBIENT,
};

enum class ShadowType {
    STATIC = 0,
    DYNAMIC,
};

constexpr std::array<ShadowType, 2> SHADOW_TYPES = {
    ShadowType::STATIC,
    ShadowType::DYNAMIC,
};

struct Point {
    Vector3 attenuation;
};

struct Directional {};

struct Spot {
    Vector3 attenuation;
    float inner_cutoff;
    float outer_cutoff;
};
struct Ambient {};

typedef union {
    Point point;
    Directional directional;
    Spot spot;
    Ambient ambient;
} Params;

class Light {
public:
    const entt::entity entity;

    bool is_on;
    bool casts_shadows;

    LightType light_type;
    ShadowType shadow_type;

    Color color;
    float intensity;

    RenderTexture2D *shadow_map;
    Matrix vp_mat;

    Params params;

    Light(
        entt::entity entity,
        LightType light_type,
        Color color,
        float intensity,
        Params params
    );

    void draw_shadow_map();

    void set_shader_uniform(Shader shader, int idx);

    nlohmann::json to_json() const;
    static Light from_json(entt::entity entity, const nlohmann::json &json_data);

private:
    bool needs_update = true;
};

std::string light_type_to_str(LightType type);
std::string shadow_type_to_str(ShadowType type);

LightType str_to_light_type(std::string str);
ShadowType str_to_shadow_type(std::string str);

}  // namespace soft_tissues::light
