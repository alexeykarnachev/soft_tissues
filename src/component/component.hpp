#pragma once

#include "entt/entity/fwd.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "transform.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"

namespace soft_tissues::component {

struct ShadowData {
    RenderTexture2D *shadow_map = nullptr;
    Matrix vp_mat = MatrixIdentity();
    bool needs_update = true;
};

// TODO: Factor out components below to their own modules when the time comes.

class Player {
public:
    nlohmann::json to_json() const {
        return nlohmann::json::object();
    }

    static Player from_json(const nlohmann::json &json_data) {
        return Player();
    }
};

class Flashlight {
public:
    nlohmann::json to_json() const {
        return nlohmann::json::object();
    }

    static Flashlight from_json(const nlohmann::json &json_data) {
        return Flashlight();
    }
};

class Parent {
public:
    entt::entity entity;

    nlohmann::json to_json() const {
        nlohmann::json json;
        json["entity"] = this->entity;
        return json;
    }

    static Parent from_json(const nlohmann::json &json_data) {
        entt::entity entity = json_data["entity"].get<entt::entity>();
        return Parent{entity};
    }
};

}  // namespace soft_tissues::component
