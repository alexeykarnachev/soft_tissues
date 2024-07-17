#include "world.hpp"

#include "camera.hpp"
#include "globals.hpp"
#include "raylib/raylib.h"
#include "tile.hpp"
#include "utils.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>

namespace soft_tissues::world {

using namespace utils;

std::array<tile::Tile, globals::WORLD_N_TILES> TILES;

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

tile::Tile *get_tile_at_cursor() {
    RayCollision collision = utils::get_cursor_floor_rect_collision(
        world::get_bound_rect(), camera::CAMERA
    );

    tile::Tile *tile = NULL;
    if (collision.hit) {
        int row = std::floor(collision.point.z);
        int col = std::floor(collision.point.x);
        uint32_t idx = row * globals::WORLD_N_COLS + col;
        tile = &TILES[idx];
    }

    return tile;
}

std::array<tile::Tile *, 4> get_tile_neighbors(uint32_t id) {
    // TODO: wtf?
    std::array<tile::Tile *, 4> neighbors = {nullptr, nullptr, nullptr, nullptr};

    int row = id / globals::WORLD_N_COLS;
    int col = id % globals::WORLD_N_COLS;

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

void set_room_tile_flags(tile::Tile *tile) {
    auto nbs = world::get_tile_neighbors(tile->get_id());
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
