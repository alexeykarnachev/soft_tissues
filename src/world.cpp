#include "world.hpp"

#include "camera.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "tile.hpp"
#include "utils.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <unordered_map>

namespace soft_tissues::world {

using namespace utils;

std::array<tile::Tile, globals::WORLD_N_TILES> TILES;

std::unordered_map<int, std::vector<tile::Tile *>> ROOM_ID_TO_TILES;
std::unordered_map<tile::Tile *, int> TILE_TO_ROOM_ID;

void load() {
    for (int i = 0; i < globals::WORLD_N_TILES; ++i) {
        TILES[i] = tile::Tile(i);
    }
}

Vector2 get_center() {
    return {0.5f * globals::WORLD_N_COLS, 0.5f * globals::WORLD_N_ROWS};
}

Rectangle get_bound_rect() {
    return {
        .x = 0.0,
        .y = 0.0,
        .width = globals::WORLD_N_COLS,
        .height = globals::WORLD_N_ROWS,
    };
}

tile::Tile *get_tile_at_row_col(int row, int col) {
    uint32_t idx = row * globals::WORLD_N_COLS + col;
    tile::Tile *tile = NULL;
    if (idx < globals::WORLD_N_TILES) {
        tile = &TILES[idx];
    }

    return tile;
}

tile::Tile *get_tile_at_cursor() {
    RayCollision collision = utils::get_cursor_floor_rect_collision(
        world::get_bound_rect(), camera::CAMERA
    );

    tile::Tile *tile = NULL;

    if (collision.hit) {
        int row = std::floor(collision.point.z);
        int col = std::floor(collision.point.x);
        tile = get_tile_at_row_col(row, col);
    }

    return tile;
}

std::pair<int, int> get_tile_row_col(tile::Tile *tile) {
    int id = tile->get_id();
    int row = id / globals::WORLD_N_COLS;
    int col = id % globals::WORLD_N_COLS;

    return {row, col};
}

std::array<tile::Tile *, 4> get_tile_neighbors(tile::Tile *tile) {
    // TODO: wtf?
    std::array<tile::Tile *, 4> neighbors = {nullptr, nullptr, nullptr, nullptr};
    auto [row, col] = get_tile_row_col(tile);

    uint32_t id = tile->get_id();

    if (row > 0) {
        neighbors[(int)CardinalDirection::NORTH] = &TILES[id - globals::WORLD_N_COLS];
    }

    if (row < (globals::WORLD_N_ROWS - 1)) {
        neighbors[(int)CardinalDirection::SOUTH] = &TILES[id + globals::WORLD_N_COLS];
    }

    if (col > 0) {
        neighbors[(int)CardinalDirection::WEST] = &TILES[id - 1];
    }

    if (col < (globals::WORLD_N_COLS - 1)) {
        neighbors[(int)CardinalDirection::EAST] = &TILES[id + 1];
    }

    return neighbors;
}

std::vector<tile::Tile *> get_tiles_between_corners(
    tile::Tile *corner_0, tile::Tile *corner_1
) {
    auto [row_0, col_0] = get_tile_row_col(corner_0);
    auto [row_1, col_1] = get_tile_row_col(corner_1);

    int row_min = std::min(row_0, row_1);
    int row_max = std::max(row_0, row_1);

    int col_min = std::min(col_0, col_1);
    int col_max = std::max(col_0, col_1);

    std::vector<tile::Tile *> tiles;
    for (int row = row_min; row <= row_max; ++row) {
        for (int col = col_min; col <= col_max; ++col) {
            tile::Tile *tile = get_tile_at_row_col(row, col);
            if (tile) tiles.push_back(tile);
        }
    }

    return tiles;
}

int add_room() {
    static int id = 0;

    ROOM_ID_TO_TILES[id] = {};

    return id++;
}

void remove_room(int room_id) {
    if (ROOM_ID_TO_TILES.count(room_id) == 0) {
        throw std::runtime_error("Can't remove unexisting room");
    }

    for (auto tile : ROOM_ID_TO_TILES[room_id]) {
        tile->flags = 0;
        TILE_TO_ROOM_ID.erase(tile);
    }

    ROOM_ID_TO_TILES.erase(room_id);
}

std::vector<int> get_room_ids() {
    std::vector<int> ids;
    for (auto &pair : ROOM_ID_TO_TILES) {
        ids.push_back(pair.first);
    }
    return ids;
}

Rectangle get_tile_rect(tile::Tile *tile) {
    auto [y, x] = get_tile_row_col(tile);
    return {.x = (float)x, .y = (float)y, .width = 1.0, .height = 1.0};
}

Vector2 get_tile_center(tile::Tile *tile) {
    auto [y, x] = get_tile_row_col(tile);
    return {.x = x + 0.5f, .y = y + 0.5f};
}

int get_tile_room_id(tile::Tile *tile) {
    if (TILE_TO_ROOM_ID.count(tile) == 0) {
        return -1;
    }

    return TILE_TO_ROOM_ID[tile];
}

std::vector<tile::Tile *> get_room_tiles(int room_id) {
    if (ROOM_ID_TO_TILES.count(room_id) == 0) {
        return {};
    }

    return ROOM_ID_TO_TILES[room_id];
}

std::vector<tile::Tile *> get_not_room_tiles(int except_room_id) {
    std::vector<tile::Tile *> tiles;
    for (auto [room_id, room_tiles] : ROOM_ID_TO_TILES) {
        if (room_id != except_room_id) {
            tiles.insert(tiles.end(), room_tiles.begin(), room_tiles.end());
        }
    }

    return tiles;
}

std::vector<tile::Tile *> get_all_rooms_tiles() {
    std::vector<tile::Tile *> tiles;
    for (auto [room_id, room_tiles] : ROOM_ID_TO_TILES) {
        tiles.insert(tiles.end(), room_tiles.begin(), room_tiles.end());
    }

    return tiles;
}

static void set_room_tile_flags(tile::Tile *tile) {
    auto nbs = world::get_tile_neighbors(tile);
    tile->flags = tile::TileFlags::TILE_FLOOR | tile::TileFlags::TILE_CEIL;

    // TODO: manual enumeration is very bad in this case, IMPROVE!
    auto nb = nbs[(int)CardinalDirection::NORTH];
    bool has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_NORTH_WALL;
    }

    nb = nbs[(int)CardinalDirection::SOUTH];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_SOUTH_WALL;
    }

    nb = nbs[(int)CardinalDirection::WEST];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_WEST_WALL;
    }

    nb = nbs[(int)CardinalDirection::EAST];
    has_nb = nb != NULL && !nb->is_empty();
    if (!has_nb) {
        tile->flags |= tile::TileFlags::TILE_EAST_WALL;
    }
}

void add_tile_to_room(tile::Tile *tile, int room_id) {
    if (tile == NULL) {
        throw std::runtime_error("Can't add NULL tile to the room");
    }

    if (!tile->is_empty()) {
        throw std::runtime_error("Can't add unempty tile to the room");
    }

    if (TILE_TO_ROOM_ID.count(tile) != 0) {
        throw std::runtime_error("Can't add already added tile to the room");
    }

    if (ROOM_ID_TO_TILES.count(room_id) == 0) {
        throw std::runtime_error("Can't add tile to unexisting room");
    }

    ROOM_ID_TO_TILES[room_id].push_back(tile);
    TILE_TO_ROOM_ID[tile] = room_id;

    set_room_tile_flags(tile);
    for (auto nb : world::get_tile_neighbors(tile)) {
        if (nb && !nb->is_empty()) {
            set_room_tile_flags(nb);
        }
    }
    set_room_tile_flags(tile);
}

void draw_grid() {
    float n_cols = globals::WORLD_N_COLS;
    float n_rows = globals::WORLD_N_ROWS;

    // z lines
    for (float x = 1.0; x < n_cols; x += 1.0) {
        Vector3 start_pos = {x, 0.0, 0.0};
        Vector3 end_pos = {x, 0.0, n_rows};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // x lines
    for (float z = 1.0; z < globals::WORLD_N_ROWS; z += 1.0) {
        Vector3 start_pos = {0.0, 0.0, z};
        Vector3 end_pos = {n_cols, 0.0, z};
        DrawLine3D(start_pos, end_pos, WHITE);
    }

    // perimiter
    DrawLine3D({0.0, 0.0, 0.0}, {n_cols, 0.0, 0.0}, RED);
    DrawLine3D({0.0, 0.0, n_rows}, {n_cols, 0.0, n_rows}, RED);

    DrawLine3D({0.0, 0.0, 0.0}, {0.0, 0.0, n_rows}, RED);
    DrawLine3D({n_cols, 0.0, 0.0}, {n_cols, 0.0, n_rows}, RED);
}

void draw_tiles() {
    for (tile::Tile &tile : TILES) {
        tile.draw();
    }
}

}  // namespace soft_tissues::world
