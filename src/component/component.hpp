#pragma once

#include "entt/entity/fwd.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "nlohmann/json.hpp"
#include "shadow_data.hpp"
#include "transform.hpp"

namespace soft_tissues::component {

// Tag and small components — move to own files when they grow.

struct Player {
    nlohmann::json to_json() const {
        return nlohmann::json::object();
    }

    static Player from_json(const nlohmann::json &json_data) {
        return Player();
    }
};

struct Flashlight {
    nlohmann::json to_json() const {
        return nlohmann::json::object();
    }

    static Flashlight from_json(const nlohmann::json &json_data) {
        return Flashlight();
    }
};

struct Parent {
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
