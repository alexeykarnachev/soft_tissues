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

std::pair<int, int> get_tile_row_col(tile::Tile *tile);
std::array<tile::Tile *, 4> get_tile_neighbors(tile::Tile *tile);
std::vector<tile::Tile *> get_tiles_between_corners(
    tile::Tile *corner_0, tile::Tile *corner_1
);

int add_room();
void remove_room(int room_id);
std::vector<int> get_room_ids();
Rectangle get_tile_rect(tile::Tile *tile);
Vector2 get_tile_center(tile::Tile *tile);
int get_tile_room_id(tile::Tile *tile);
std::vector<tile::Tile *> get_room_tiles(int room_id);
std::vector<tile::Tile *> get_not_room_tiles(int except_room_id);
std::vector<tile::Tile *> get_all_rooms_tiles();

void add_tile_to_room(tile::Tile *tile, int room_id);

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
