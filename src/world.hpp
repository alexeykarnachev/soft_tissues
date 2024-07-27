#pragma once

#include "raylib/raylib.h"
#include "tile.hpp"
#include <array>
#include <vector>

namespace soft_tissues::world {

static constexpr int HEIGHT = 3;
static constexpr int N_ROWS = 5;
static constexpr int N_COLS = 5;
static constexpr int N_TILES = N_ROWS * N_COLS;
static constexpr Vector2 SIZE = {N_ROWS, N_COLS};
static constexpr Vector2 ORIGIN = {3.0, 3.0};

void load();

std::pair<int, int> get_row_col_at_position(Vector2 pos);
Rectangle get_bound_rect();

tile::Tile *get_tile_at_row_col(int row, int col);
tile::Tile *get_tile_at_position(Vector2 pos);
tile::Tile *get_tile_at_cursor(Vector2 *out_pos = NULL);
tile::Tile *get_nearest_tile_neighbor_at_position(Vector2 pos);

std::pair<int, int> get_tile_row_col(tile::Tile *tile);
std::array<tile::Tile *, 4> get_tile_neighbors(tile::Tile *tile);
std::vector<tile::Tile *> get_tiles_between_corners(
    tile::Tile *corner_0, tile::Tile *corner_1
);

int add_room();

void remove_room(int room_id);
void clear_tile(tile::Tile *tile);
void remove_wall_between_neighbor_tiles(tile::Tile *tile0, tile::Tile *tile1);

std::vector<int> get_room_ids();
int get_tile_room_id(tile::Tile *tile);
std::vector<tile::Tile *> get_room_tiles(int room_id);
std::vector<tile::Tile *> get_not_room_tiles(int except_room_id);
std::vector<tile::Tile *> get_all_rooms_tiles();

void set_room_tile_materials(int room_id, tile::TileMaterials materials);

void add_tile_to_room(tile::Tile *tile, int room_id);

void draw_grid();
void draw_tiles();

}  // namespace soft_tissues::world
