#pragma once

#include "raylib/raylib.h"
#include "utils.hpp"
#include "nlohmann/json.hpp"
#include <array>
#include <cstdint>
#include <string>

namespace soft_tissues::tile {

using utils::Direction;

class TileMaterials {
public:
    std::string floor_key;
    std::string wall_key;
    std::string ceil_key;

    TileMaterials();
    TileMaterials(std::string material_pbr_key);
    TileMaterials(std::string floor_key, std::string wall_key, std::string ceil_key);

    nlohmann::json to_json() const;
    static TileMaterials from_json(const nlohmann::json &json_data);
};

enum class TileWall {
    NONE,
    DOOR,
    SOLID,
};

class Tile {
private:
    uint32_t id = 0;
    std::array<TileWall, 4> walls = {TileWall::NONE};

public:
    TileMaterials materials;
    Color constant_color = {0, 0, 0, 0};

    Tile();
    Tile(uint32_t id);
    Tile(
        uint32_t id,
        std::array<TileWall, 4> walls,
        TileMaterials materials,
        Color constant_color
    );

    uint32_t get_id() const;

    void remove_wall(Direction direction);
    void remove_all_walls();

    void set_solid_wall(Direction direction);
    void set_door_wall(Direction direction);

    bool has_any_wall(Direction direction) const;
    bool has_solid_wall(Direction direction) const;
    bool has_door_wall(Direction direction) const;
    bool has_any_wall() const;
    bool has_solid_wall() const;
    bool has_door_wall() const;

    Vector2 get_floor_position() const;
    Matrix get_floor_matrix() const;
    Matrix get_ceil_matrix() const;
    Matrix get_wall_matrix(Direction direction, int elevation) const;

    nlohmann::json to_json() const;
    static Tile from_json(const nlohmann::json &json_data);
};

}  // namespace soft_tissues::tile
