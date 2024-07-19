#pragma once

#include "raylib/raylib.h"
#include "tile.hpp"
#include <array>
#include <cstdint>
#include <unordered_set>

namespace soft_tissues::world {

void load();

Vector2 get_center();
Rectangle get_bound_rect();
tile::Tile *get_tile_at(int row, int col);
tile::Tile *get_tile_at_cursor();
std::array<tile::Tile *, 4> get_tile_neighbors(uint32_t id);

std::unordered_set<tile::Tile *> get_tiles_between_corners(
    uint32_t corner_0, uint32_t corner_1
);

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
