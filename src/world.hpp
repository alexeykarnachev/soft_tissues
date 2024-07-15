#pragma once

#include "raylib/raylib.h"
#include "tile.hpp"

namespace soft_tissues::world {

void load();

Vector2 get_center();
Rectangle get_bound_rect();
tile::Tile *get_tile_at_cursor();

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
