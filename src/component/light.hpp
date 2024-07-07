#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::light {

enum class Type {
    POINT = 0,
    DIRECTIONAL,
};

struct Point {
    Vector3 attenuation;
};

struct Directional {
    Vector3 direction;
};

typedef union {
    Point point;
    Directional directional;
} Params;

class Light {
public:
    const entt::entity entity;
    const Type type;

    Color color;
    float intensity;
    Params params;

    Light(entt::entity entity, Type type, Color color, float intensity, Params params);

    void set_shader_uniform(Shader shader, int idx);
};

}  // namespace soft_tissues::light
