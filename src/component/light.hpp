#pragma once

#include "nlohmann/json.hpp"
#include "raylib/raylib.h"
#include <array>
#include <string>

namespace soft_tissues::component {

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

struct PointParams {
    Vector3 attenuation;
};

struct DirectionalParams {};

struct SpotParams {
    Vector3 attenuation;
    float inner_cutoff;
    float outer_cutoff;
};

struct AmbientParams {};

typedef union {
    PointParams point;
    DirectionalParams directional;
    SpotParams spot;
    AmbientParams ambient;
} LightParams;

struct Light {
    bool is_on;
    bool casts_shadows;

    LightType light_type;
    ShadowType shadow_type;

    Color color;
    float intensity;

    LightParams params;

    Light(
        LightType light_type,
        Color color,
        float intensity,
        LightParams params
    );

    nlohmann::json to_json() const;
    static Light from_json(const nlohmann::json &json_data);
};

std::string light_type_to_str(LightType type);
std::string shadow_type_to_str(ShadowType type);

LightType str_to_light_type(std::string str);
ShadowType str_to_shadow_type(std::string str);

}  // namespace soft_tissues::component
