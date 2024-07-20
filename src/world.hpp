#pragma once

#include "raylib/raylib.h"
#include "tile.hpp"
#include <array>
#include <vector>

namespace soft_tissues::world {

void load();

Vector2 get_center();
Rectangle get_bound_rect();

tile::Tile *get_tile_at_row_col(int row, int col);
tile::Tile *get_tile_at_cursor();

std::array<tile::Tile *, 4> get_tile_neighbors(tile::Tile *tile);
std::vector<tile::Tile *> get_tiles_between_corners(
    tile::Tile *corner_0, tile::Tile *corner_1
);

int add_room();
std::vector<int> get_room_ids();
std::vector<tile::Tile *> get_room_tiles(int room_id);

void add_tile_to_room(tile::Tile *tile, int room_id);

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
