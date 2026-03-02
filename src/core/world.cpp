#include "world.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "tile.hpp"
#include "utils.hpp"
#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>

namespace soft_tissues::world {

using namespace utils;

static constexpr int N_TILES = N_ROWS * N_COLS;
static const Vector2 SIZE = {N_COLS, N_ROWS};

static std::array<tile::Tile, N_TILES> TILES;
static std::unordered_map<int, std::vector<tile::Tile *>> ROOM_ID_TO_TILES;
static std::unordered_map<tile::Tile *, int> TILE_TO_ROOM_ID;

void reset() {
    for (int i = 0; i < N_TILES; ++i) {
        TILES[i] = tile::Tile(i);
    }

    ROOM_ID_TO_TILES.clear();
    TILE_TO_ROOM_ID.clear();
}

tile::Tile *get_tiles() {
    return TILES.data();
}

int get_tiles_count() {
    return N_TILES;
}

int get_rooms_count() {
    return ROOM_ID_TO_TILES.size();
}

Vector2 get_size() {
    return SIZE;
}

std::pair<int, int> get_row_col_at_position(Vector2 pos) {
    Vector2 a = Vector2Subtract(pos, ORIGIN);
    Vector2 b = Vector2Add(a, Vector2Scale(SIZE, 0.5));

    int col = std::floor(b.x);
    int row = std::floor(b.y);

    return {row, col};
}

Rectangle get_bound_rect() {
    Vector2 top_left = Vector2Subtract(ORIGIN, Vector2Scale(SIZE, 0.5));

    return {
        .x = top_left.x,
        .y = top_left.y,
        .width = N_COLS,
        .height = N_ROWS,
    };
}

tile::Tile *get_tile_at_row_col(int row, int col) {
    if (row < 0 || row >= N_ROWS || col < 0 || col >= N_COLS) return nullptr;

    int idx = row * N_COLS + col;
    return &TILES[idx];
}

tile::Tile *get_tile_at_position(Vector2 pos) {
    auto [row, col] = get_row_col_at_position(pos);
    return get_tile_at_row_col(row, col);
}

tile::Tile *get_tile_at_cursor(Camera3D camera, Vector2 *out_pos) {
    Rectangle rect = world::get_bound_rect();
    RayCollision collision = utils::get_cursor_floor_rect_collision(rect, camera);

    tile::Tile *tile = nullptr;

    if (collision.hit) {
        Vector2 point = {collision.point.x, collision.point.z};
        auto [row, col] = get_row_col_at_position(point);
        tile = get_tile_at_row_col(row, col);

        if (out_pos != nullptr) *out_pos = point;
    }

    return tile;
}

tile::Tile *get_nearest_tile_neighbor_at_position(Vector2 pos) {
    auto tile = get_tile_at_position(pos);
    if (tile == nullptr) return nullptr;

    tile::Tile *nearest_nb = nullptr;

    float nearest_dist = FLT_MAX;
    for (auto nb : get_tile_neighbors(tile)) {
        if (nb == nullptr) continue;

        float dist = Vector2DistanceSqr(pos, nb->get_floor_position());
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_nb = nb;
        }
    }

    return nearest_nb;
}

std::pair<int, int> get_tile_row_col(tile::Tile *tile) {
    Vector2 pos = tile->get_floor_position();
    return get_row_col_at_position(pos);
}

std::array<tile::Tile *, 4> get_tile_neighbors(tile::Tile *tile) {
    std::array<tile::Tile *, 4> neighbors = {nullptr};
    auto [row, col] = get_tile_row_col(tile);

    uint32_t id = tile->get_id();

    if (row > 0) {
        neighbors[(int)Direction::NORTH] = &TILES[id - N_COLS];
    }

    if (row < (N_ROWS - 1)) {
        neighbors[(int)Direction::SOUTH] = &TILES[id + N_COLS];
    }

    if (col > 0) {
        neighbors[(int)Direction::WEST] = &TILES[id - 1];
    }

    if (col < (N_COLS - 1)) {
        neighbors[(int)Direction::EAST] = &TILES[id + 1];
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
    for (auto &[room_id, room_tiles] : ROOM_ID_TO_TILES) {
        if (room_tiles.empty()) return room_id;
    }

    int id = 0;
    for (auto &[room_id, _] : ROOM_ID_TO_TILES) {
        if (room_id >= id) id = room_id + 1;
    }
    ROOM_ID_TO_TILES[id] = {};
    return id;
}

void remove_room(int room_id) {
    if (ROOM_ID_TO_TILES.count(room_id) == 0) {
        throw std::runtime_error("Can't remove unexisting room");
    }

    for (auto tile : ROOM_ID_TO_TILES[room_id]) {
        tile->remove_all_walls();
        TILE_TO_ROOM_ID.erase(tile);
    }

    ROOM_ID_TO_TILES.erase(room_id);
}

static void fix_tile_walls(tile::Tile *tile) {
    if (tile == nullptr) return;

    int room_id = get_tile_room_id(tile);
    if (room_id == -1) return;

    auto neighbors = world::get_tile_neighbors(tile);

    for (size_t i = 0; i < neighbors.size(); ++i) {
        Direction tile_direction = (Direction)i;
        Direction nb_direction = flip_direction(tile_direction);
        auto nb = neighbors[i];

        if (nb == nullptr || get_tile_room_id(nb) == -1) {
            tile->set_solid_wall(tile_direction);
        } else {
            bool is_door_between = tile->has_door_wall(tile_direction)
                                   && nb->has_door_wall(nb_direction);
            int nb_room_id = get_tile_room_id(nb);
            bool is_same_room = room_id == nb_room_id;

            if (!is_same_room && !is_door_between) {
                tile->set_solid_wall(tile_direction);
            } else if (is_same_room) {
                tile->remove_wall(tile_direction);
            }
        }
    }
}

void clear_tile(tile::Tile *tile) {
    tile->remove_all_walls();

    if (TILE_TO_ROOM_ID.count(tile) != 0) {
        int room_id = TILE_TO_ROOM_ID[tile];

        auto &tiles = ROOM_ID_TO_TILES[room_id];
        tiles.erase(std::remove(tiles.begin(), tiles.end(), tile), tiles.end());

        TILE_TO_ROOM_ID.erase(tile);
    }

    for (auto nb : world::get_tile_neighbors(tile)) {
        fix_tile_walls(nb);
    }
}

void set_door_between_neighbor_tiles(tile::Tile *tile0, tile::Tile *tile1) {
    auto [row0, col0] = get_tile_row_col(tile0);
    auto [row1, col1] = get_tile_row_col(tile1);

    if (row0 - 1 == row1 && col0 == col1) {
        tile0->set_door_wall(Direction::NORTH);
        tile1->set_door_wall(Direction::SOUTH);
    } else if (row0 == row1 && col0 + 1 == col1) {
        tile0->set_door_wall(Direction::EAST);
        tile1->set_door_wall(Direction::WEST);
    } else if (row0 + 1 == row1 && col0 == col1) {
        tile0->set_door_wall(Direction::SOUTH);
        tile1->set_door_wall(Direction::NORTH);
    } else if (row0 == row1 && col0 - 1 == col1) {
        tile0->set_door_wall(Direction::WEST);
        tile1->set_door_wall(Direction::EAST);
    } else {
        TraceLog(LOG_WARNING, "Can't set a door between non-neigbor tiles");
    }
}

std::vector<int> get_room_ids() {
    std::vector<int> ids;
    for (auto &pair : ROOM_ID_TO_TILES) {
        ids.push_back(pair.first);
    }
    return ids;
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
    for (auto &[room_id, room_tiles] : ROOM_ID_TO_TILES) {
        if (room_id != except_room_id) {
            tiles.insert(tiles.end(), room_tiles.begin(), room_tiles.end());
        }
    }

    return tiles;
}

std::vector<tile::Tile *> get_all_rooms_tiles() {
    std::vector<tile::Tile *> tiles;
    for (auto &[room_id, room_tiles] : ROOM_ID_TO_TILES) {
        tiles.insert(tiles.end(), room_tiles.begin(), room_tiles.end());
    }

    return tiles;
}

void set_room_tile_materials(int room_id, tile::TileMaterials materials) {
    for (auto tile : get_room_tiles(room_id)) {
        tile->materials = materials;
    }
}

void add_tile_to_room(tile::Tile *tile, int room_id) {
    if (tile == nullptr) {
        throw std::runtime_error("Can't add nullptr tile to the room");
    }

    if (TILE_TO_ROOM_ID.count(tile) != 0) {
        throw std::runtime_error("Can't add already added tile to the room");
    }

    if (ROOM_ID_TO_TILES.count(room_id) == 0) {
        throw std::runtime_error("Can't add tile to unexisting room");
    }

    ROOM_ID_TO_TILES[room_id].push_back(tile);
    TILE_TO_ROOM_ID[tile] = room_id;

    fix_tile_walls(tile);
    for (auto nb : world::get_tile_neighbors(tile)) {
        fix_tile_walls(nb);
    }
}

void clear_state() {
    reset();
}

std::vector<std::pair<tile::Tile *, int>> get_tiles_with_room_ids() {
    std::vector<std::pair<tile::Tile *, int>> result;
    for (const auto &[tile, room_id] : TILE_TO_ROOM_ID) {
        result.push_back({tile, room_id});
    }
    return result;
}

void load_tile_to_room(tile::Tile tile, int room_id) {
    int tile_id = tile.get_id();
    if (tile_id < 0 || tile_id >= N_TILES) {
        throw std::runtime_error("load_tile_to_room: tile_id out of bounds");
    }
    TILES[tile_id] = tile;
    TILE_TO_ROOM_ID[&TILES[tile_id]] = room_id;
    ROOM_ID_TO_TILES[room_id].push_back(&TILES[tile_id]);
}

}  // namespace soft_tissues::world
