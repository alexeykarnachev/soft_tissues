#pragma once

#include "raylib/raylib.h"
#include <cstdint>

namespace soft_tissues::tiling {

// -----------------------------------------------------------------------
// tile flags
enum TileFlags : uint16_t {
    TILE_FLOOR = 1 << 0,
    TILE_CEIL = 1 << 1,
    TILE_NORTH_WALL = 1 << 2,
    TILE_SOUTH_WALL = 1 << 3,
    TILE_WEST_WALL = 1 << 4,
    TILE_EAST_WALL = 1 << 5,
    TILE_NORTH_DOOR = 1 << 6,
    TILE_SOUTH_DOOR = 1 << 7,
    TILE_WEST_DOOR = 1 << 8,
    TILE_EAST_DOOR = 1 << 9,
    TILE_FLOOR_HATCH = 1 << 10,
    TILE_CEIL_HATCH = 1 << 11
};

// -----------------------------------------------------------------------
// tile materials
class TileMaterials {
public:
    Material floor;
    Material wall;
    Material ceil;

    TileMaterials(Material floor, Material wall, Material ceil);
};

class Tile {
private:
    uint32_t idx;
    uint16_t flags;
    TileMaterials materials;

    static constexpr int height = 3;

public:
    Tile(uint32_t idx, uint16_t flags, TileMaterials materials);

    bool has_flags(uint16_t flags);
    Vector2 get_world_position();

    void draw();
};

}  // namespace soft_tissues::tiling
