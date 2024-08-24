#include "tile.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "utils.hpp"
#include "world.hpp"
#include <cstdint>

namespace soft_tissues::tile {

using namespace utils;

TileMaterials::TileMaterials() = default;

TileMaterials::TileMaterials(std::string material_pbr_key)
    : floor_key(material_pbr_key)
    , wall_key(material_pbr_key)
    , ceil_key(material_pbr_key) {}

TileMaterials::TileMaterials(
    std::string floor_key, std::string wall_key, std::string ceil_key
)
    : floor_key(floor_key)
    , wall_key(wall_key)
    , ceil_key(ceil_key) {}

Tile::Tile() = default;

Tile::Tile(uint32_t id)
    : id(id) {}

uint32_t Tile::get_id() {
    return this->id;
}

void Tile::remove_wall(Direction direction) {
    this->walls[direction] = TileWall::NONE;
}

void Tile::remove_all_walls() {
    this->remove_wall(Direction::NORTH);
    this->remove_wall(Direction::SOUTH);
    this->remove_wall(Direction::WEST);
    this->remove_wall(Direction::EAST);
}

void Tile::set_solid_wall(Direction direction) {
    this->walls[direction] = TileWall::SOLID;
}

void Tile::set_door_wall(Direction direction) {
    this->walls[direction] = TileWall::DOOR;
}

bool Tile::has_any_wall(Direction direction) {
    return this->walls[direction] != TileWall::NONE;
}

bool Tile::has_solid_wall(Direction direction) {
    return this->walls[direction] == TileWall::SOLID;
}

bool Tile::has_door_wall(Direction direction) {
    return this->walls[direction] == TileWall::DOOR;
}

bool Tile::has_any_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_any_wall((Direction)i)) return true;
    }
    return false;
}

bool Tile::has_solid_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_solid_wall((Direction)i)) return true;
    }
    return false;
}

bool Tile::has_door_wall() {
    for (int i = 0; i < 4; ++i) {
        if (this->has_door_wall((Direction)i)) return true;
    }
    return false;
}

Vector2 Tile::get_floor_position() {
    uint32_t row = this->id / world::N_COLS;
    uint32_t col = this->id % world::N_COLS;

    Vector2 pos = {col + 0.5f, row + 0.5f};
    pos = Vector2Subtract(pos, Vector2Scale(world::get_size(), 0.5));
    pos = Vector2Add(pos, world::ORIGIN);

    return pos;
}

Matrix Tile::get_floor_matrix() {
    Vector2 position = this->get_floor_position();
    Matrix matrix = MatrixTranslate(position.x, 0.0, position.y);

    return matrix;
}

Matrix Tile::get_ceil_matrix() {
    Vector2 position = this->get_floor_position();

    Matrix t = MatrixTranslate(position.x, world::HEIGHT, position.y);
    Matrix r = MatrixRotateX(PI);
    Matrix matrix = MatrixMultiply(r, t);

    return matrix;
}

Matrix Tile::get_wall_matrix(Direction direction, int elevation) {
    Vector2 position = this->get_floor_position();
    float y = elevation + 0.5;

    Matrix matrix;
    switch (direction) {
        case Direction::NORTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y - 0.5);
            Matrix r = MatrixRotateX(0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::SOUTH: {
            Matrix t = MatrixTranslate(position.x, y, position.y + 0.5);
            Matrix r = MatrixRotateX(-0.5 * PI);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::WEST: {
            Matrix t = MatrixTranslate(position.x - 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
        case Direction::EAST: {
            Matrix t = MatrixTranslate(position.x + 0.5, y, position.y);
            Matrix rx = MatrixRotateX(0.5 * PI);
            Matrix ry = MatrixRotateY(-0.5 * PI);
            Matrix r = MatrixMultiply(rx, ry);
            matrix = MatrixMultiply(r, t);
        } break;
    }

    return matrix;
}

}  // namespace soft_tissues::tile
