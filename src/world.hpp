#pragma once

#include "globals.hpp"
#include "raylib/raylib.h"
#include <array>
#include <vector>

namespace soft_tissues::world {

// -----------------------------------------------------------------------
// tile flags
enum TileFlags : uint16_t {
    TILE_FLOOR = 1 << 0,
    TILE_CEIL = 1 << 1,
    TILE_NORTH_WALL = 1 << 2,
    TILE_SOUTH_WALL = 1 << 3,
    TILE_WEST_WALL = 1 << 4,
    TILE_EAST_WALL = 1 << 5
};

// -----------------------------------------------------------------------
// tile materials
class TileMaterials {
public:
    Material floor;
    Material wall;
    Material ceil;

    TileMaterials();
    TileMaterials(Material floor, Material wall, Material ceil);
};

class Tile {
public:
    uint16_t flags;
    TileMaterials materials;

    Tile();
    Tile(uint16_t flags, TileMaterials materials);

    bool has_flags(uint16_t flags);
};

void load();
void unload();

void draw_tiles();

}  // namespace soft_tissues::world
