#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include "utils.hpp"
#include <array>
#include <cstdint>

namespace soft_tissues::tile {

using namespace utils;

class TileMaterials {
public:
    pbr::MaterialPBR floor;
    pbr::MaterialPBR wall;
    pbr::MaterialPBR ceil;

    TileMaterials();
    TileMaterials(pbr::MaterialPBR material);
    TileMaterials(pbr::MaterialPBR floor, pbr::MaterialPBR wall, pbr::MaterialPBR ceil);
};

enum class TileWall {
    NONE,
    DOOR,
    SOLID,
};

class Tile {
private:
    uint32_t id;
    std::array<TileWall, 4> walls = {TileWall::NONE};

public:
    TileMaterials materials;
    Color constant_color = {0, 0, 0, 0};

    Tile();
    Tile(uint32_t id);

    uint32_t get_id();

    void remove_wall(Direction direction);
    void remove_all_walls();

    void set_solid_wall(Direction direction);
    void set_door_wall(Direction direction);

    bool has_any_wall(Direction direction);
    bool has_solid_wall(Direction direction);
    bool has_door_wall(Direction direction);
    bool has_any_wall();
    bool has_solid_wall();
    bool has_door_wall();

    Vector2 get_floor_position();
    Matrix get_floor_matrix();
    Matrix get_ceil_matrix();
    Matrix get_wall_matrix(Direction direction, int elevation);
};

}  // namespace soft_tissues::tile
