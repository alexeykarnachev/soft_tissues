#pragma once

#include "entt/entity/fwd.hpp"
#include "nlohmann/json.hpp"
#include "raylib/raylib.h"

namespace nlohmann {

template <> struct adl_serializer<Vector3> {
    static void to_json(json &j, const Vector3 &v) {
        j = {v.x, v.y, v.z};
    }

    static void from_json(const json &j, Vector3 &v) {
        v.x = j[0].get<float>();
        v.y = j[1].get<float>();
        v.z = j[2].get<float>();
    }
};

template <> struct adl_serializer<Color> {
    static void to_json(json &j, const Color &c) {
        j = {c.r, c.g, c.b, c.a};
    }

    static void from_json(const json &j, Color &c) {
        c.r = j[0].get<unsigned char>();
        c.g = j[1].get<unsigned char>();
        c.b = j[2].get<unsigned char>();
        c.a = j[3].get<unsigned char>();
    }
};

template <> struct adl_serializer<entt::entity> {
    static void to_json(json &j, const entt::entity &e) {
        j = static_cast<uint32_t>(e);
    }

    static void from_json(const json &j, entt::entity &e) {
        e = static_cast<entt::entity>(j);
    }
};

}  // namespace nlohmann
