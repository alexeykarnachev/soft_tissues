#pragma once

#include "pbr.hpp"
#include "raylib/raylib.h"
#include "utils.hpp"
#include <cstdint>

namespace soft_tissues::tile {

using namespace utils;

enum TileFlags : uint16_t {
    TILE_FLOOR = 1 << 0,
    TILE_CEIL = 1 << 1,
    TILE_NORTH_WALL = 1 << 2,
    TILE_SOUTH_WALL = 1 << 3,
    TILE_WEST_WALL = 1 << 4,
    TILE_EAST_WALL = 1 << 5
};

class TileMaterials {
public:
    pbr::MaterialPBR floor;
    pbr::MaterialPBR wall;
    pbr::MaterialPBR ceil;

    TileMaterials();
    TileMaterials(pbr::MaterialPBR material);
    TileMaterials(pbr::MaterialPBR floor, pbr::MaterialPBR wall, pbr::MaterialPBR ceil);
};

class Tile {
private:
    uint32_t id;

public:
    uint16_t flags = 0;
    TileMaterials materials;
    Color constant_color = {0, 0, 0, 0};

    Tile();
    Tile(uint32_t id);

    uint32_t get_id();
    bool is_empty();
    bool has_flags(uint16_t flags);

    Vector2 get_floor_position();
    Matrix get_floor_matrix();
    Matrix get_ceil_matrix();
    Matrix get_wall_matrix(CardinalDirection side, int elevation);

    void draw();
};

}  // namespace soft_tissues::tile
