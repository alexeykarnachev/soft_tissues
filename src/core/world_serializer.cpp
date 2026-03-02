#include "world_serializer.hpp"

#include "component/component.hpp"
#include "globals.hpp"
#include "tile.hpp"
#include "world.hpp"
#include "nlohmann/json.hpp"
#include "raylib/raylib.h"
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace soft_tissues::world_serializer {

void save(const std::string &file_path) {
    nlohmann::json json;

    // -------------------------------------------------------------------
    // TILES
    json["tiles"] = nlohmann::json::array();
    for (const auto &[tile, room_id] : world::get_tiles_with_room_ids()) {
        nlohmann::json tile_json = tile->to_json();
        tile_json["room_id"] = room_id;
        json["tiles"].push_back(tile_json);
    }

    // -------------------------------------------------------------------
    // ENTITIES
    json["entities"] = nlohmann::json::array();
    auto view = globals::registry.view<entt::entity>();
    for (auto entity : view) {
        nlohmann::json entity_json;

        // id
        entity_json["id"] = (uint32_t)entity;

        // Transform
        if (globals::registry.all_of<component::Transform>(entity)) {
            auto &c = globals::registry.get<component::Transform>(entity);
            entity_json["Transform"] = c.to_json();
        }

        // MyMesh
        if (globals::registry.all_of<component::MyMesh>(entity)) {
            auto &c = globals::registry.get<component::MyMesh>(entity);
            entity_json["MyMesh"] = c.to_json();
        }

        // Parent
        if (globals::registry.all_of<component::Parent>(entity)) {
            auto &c = globals::registry.get<component::Parent>(entity);
            entity_json["Parent"] = c.to_json();
        }

        // Light
        if (globals::registry.all_of<component::Light>(entity)) {
            auto &c = globals::registry.get<component::Light>(entity);
            entity_json["Light"] = c.to_json();
        }

        // Player
        if (globals::registry.all_of<component::Player>(entity)) {
            entity_json["Player"] = component::Player().to_json();
        }

        // Flashlight
        if (globals::registry.all_of<component::Flashlight>(entity)) {
            entity_json["Flashlight"] = component::Flashlight().to_json();
        }

        json["entities"].push_back(entity_json);
    }

    // -------------------------------------------------------------------
    // save file
    std::ofstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + file_path);
    }

    file << json.dump(4);

    if (file.fail()) {
        throw std::runtime_error("Failed to write to file: " + file_path);
    }

    file.close();
}

void load(const std::string &file_path) {
    // parse file before clearing state — a failed parse must not wipe the world
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + file_path);
    }

    nlohmann::json json;
    file >> json;
    file.close();

    globals::registry.clear();
    world::clear_state();

    // -------------------------------------------------------------------
    // TILES
    for (const auto &tile_json : json["tiles"]) {
        tile::Tile tile = tile::Tile::from_json(tile_json);
        int room_id = tile_json["room_id"];
        world::load_tile_to_room(tile, room_id);
    }

    // -------------------------------------------------------------------
    // ENTITIES
    std::unordered_map<uint32_t, entt::entity> id_map;
    std::vector<std::pair<entt::entity, uint32_t>> parents_to_assign;

    for (const auto &entity_json : json["entities"]) {
        uint32_t old_id = entity_json["id"].get<uint32_t>();
        auto new_entity = globals::registry.create();
        id_map[old_id] = new_entity;

        // Transform
        if (entity_json.contains("Transform")) {
            auto transform = component::Transform::from_json(
                entity_json["Transform"]
            );
            globals::registry.emplace<component::Transform>(new_entity, transform);
        }

        // MyMesh
        if (entity_json.contains("MyMesh")) {
            auto my_mesh = component::MyMesh::from_json(entity_json["MyMesh"]);
            globals::registry.emplace<component::MyMesh>(new_entity, my_mesh);
        }

        // Parent
        if (entity_json.contains("Parent")) {
            auto parent_json = entity_json["Parent"];
            uint32_t parent_old_id = parent_json["entity"].get<uint32_t>();
            parents_to_assign.push_back({new_entity, parent_old_id});
        }

        // Light
        if (entity_json.contains("Light")) {
            auto light = component::Light::from_json(entity_json["Light"]);
            globals::registry.emplace<component::Light>(new_entity, std::move(light));
        }

        // Player
        if (entity_json.contains("Player")) {
            globals::registry.emplace<component::Player>(new_entity);
        }

        // Flashlight
        if (entity_json.contains("Flashlight")) {
            globals::registry.emplace<component::Flashlight>(new_entity);
        }
    }

    // assign parents after all entities have been created
    for (const auto &[child_entity, parent_old_id] : parents_to_assign) {
        if (id_map.count(parent_old_id) == 0) {
            TraceLog(LOG_WARNING, "Skipping parent assignment: entity %u not found", parent_old_id);
            continue;
        }
        entt::entity parent_entity = id_map[parent_old_id];
        globals::registry.emplace<component::Parent>(
            child_entity, component::Parent{parent_entity}
        );
    }
}

}  // namespace soft_tissues::world_serializer
