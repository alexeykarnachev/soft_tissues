#pragma once

#include "raylib/raylib.h"
#include <cstdint>

namespace soft_tissues::tile {

enum class CardinalDirection {
    NORTH,
    SOUTH,
    WEST,
    EAST,
};

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
    Material floor;
    Material wall;
    Material ceil;

    TileMaterials();
    TileMaterials(Material floor, Material wall, Material ceil);
};

class Tile {
private:
    uint32_t idx;

public:
    uint16_t flags = 0;
    TileMaterials materials;

    Tile();
    Tile(uint32_t idx);

    bool has_flags(uint16_t flags);

    Vector2 get_floor_position();
    Matrix get_floor_matrix();
    Matrix get_ceil_matrix();
    Matrix get_wall_matrix(CardinalDirection side, int elevation);

    void draw();
};

}  // namespace soft_tissues::tile
