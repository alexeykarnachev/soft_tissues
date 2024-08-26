#include "tile.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "utils.hpp"
#include "world.hpp"
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>

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

std::string TileMaterials::to_string() {
    std::string str;

    str.append(this->floor_key + " ");
    str.append(this->wall_key + " ");
    str.append(this->ceil_key);

    return str;
}

TileMaterials TileMaterials::from_string(std::string str) {
    std::istringstream iss(str);

    std::string floor_key;
    std::string wall_key;
    std::string ceil_key;

    if (!(iss >> floor_key >> wall_key >> ceil_key)) {
        auto err_str = "Failed to create TileMaterials from string: " + str;
        throw std::runtime_error(err_str);
    }

    return TileMaterials(floor_key, wall_key, ceil_key);
}

Tile::Tile() = default;

Tile::Tile(uint32_t id)
    : id(id) {}

Tile::Tile(
    uint32_t id,
    std::array<TileWall, 4> walls,
    TileMaterials materials,
    Color constant_color
)
    : id(id)
    , walls(walls)
    , materials(materials)
    , constant_color(constant_color) {}

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

std::string Tile::to_string() {
    std::string str;

    // id
    str.append(std::to_string(this->id) + " ");

    // walls
    str.append(std::to_string((int)this->walls[0]) + " ");
    str.append(std::to_string((int)this->walls[1]) + " ");
    str.append(std::to_string((int)this->walls[2]) + " ");
    str.append(std::to_string((int)this->walls[3]) + " ");

    // materials
    str.append(this->materials.to_string() + " ");

    // constant color
    str.append(std::to_string((int)this->constant_color.r) + " ");
    str.append(std::to_string((int)this->constant_color.g) + " ");
    str.append(std::to_string((int)this->constant_color.b) + " ");
    str.append(std::to_string((int)this->constant_color.a));

    return str;
}

Tile Tile::from_string(std::string str) {
    std::istringstream iss(str);
    uint32_t id;
    std::array<TileWall, 4> walls;
    std::string material;
    std::string materials_str;
    uint8_t r, g, b, a;

    // Parse id
    if (!(iss >> id)) {
        throw std::runtime_error("Failed to parse id from string: " + str);
    }

    // Parse walls
    for (auto &wall : walls) {
        int wall_int;
        if (!(iss >> wall_int)) {
            throw std::runtime_error("Failed to parse wall from string: " + str);
        }
        wall = static_cast<TileWall>(wall_int);
    }

    // Parse materials
    for (int i = 0; i < 3; ++i) {
        if (!(iss >> material)) {
            throw std::runtime_error("Failed to parse TileMaterials from string: " + str);
        }
        if (i > 0) materials_str += " ";
        materials_str += material;
    }
    TileMaterials tile_materials = TileMaterials::from_string(materials_str);

    // Parse constant color
    if (!(iss >> r >> g >> b >> a)) {
        throw std::runtime_error("Failed to parse Color from string: " + str);
    }
    Color constant_color{r, g, b, a};

    return Tile(id, walls, tile_materials, constant_color);
}

}  // namespace soft_tissues::tile
