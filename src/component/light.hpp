#pragma once

#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"

namespace soft_tissues::light {

enum class Type {
    POINT = 0,
    DIRECTIONAL,
    SPOT,
    AMBIENT,
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
    Type type;

    Color color;
    float intensity;
    Params params;

    Light(entt::entity entity, Type type, Color color, float intensity, Params params);

    void set_shader_uniform(Shader shader, int idx);
};

std::string get_type_name(Type type);

}  // namespace soft_tissues::light
