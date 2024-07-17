#pragma once

#include "raylib/raylib.h"
#include "tile.hpp"
#include <array>
#include <cstdint>

namespace soft_tissues::world {

void load();

Vector2 get_center();
Rectangle get_bound_rect();
tile::Tile *get_tile_at_cursor();
std::array<tile::Tile *, 4> get_tile_neighbors(uint32_t id);

void set_room_tile_flags(tile::Tile *tile);

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
