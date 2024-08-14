#include "../utils.hpp"

#include "../world.hpp"
#include <stdexcept>

namespace soft_tissues::editor::utils {

using namespace soft_tissues::utils;

static void draw_tile_perimiter(tile::Tile *tile, Color solid_color, Color door_color) {
    static float e = 3e-3;
    static float w = 0.2;
    static float h = w + 1.0;

    if (tile == NULL) {
        throw std::runtime_error("Can't draw tile perimiter for the NULL tile");
    };

    Vector2 center = tile->get_floor_position();

    // TODO: Refactor these:

    // north
    Vector3 pos = {center.x, e, center.y - 0.5f};
    Vector2 size = {h, w};

    if (tile->has_solid_wall(Direction::NORTH)) {
        DrawPlane(pos, size, solid_color);
    } else if (tile->has_door_wall(Direction::NORTH)) {
        pos.y += e;
        DrawPlane(pos, size, door_color);
    }

    // south
    pos = {center.x, e, center.y + 0.5f};
    size = {h, w};

    if (tile->has_solid_wall(Direction::SOUTH)) {
        DrawPlane(pos, size, solid_color);
    } else if (tile->has_door_wall(Direction::SOUTH)) {
        pos.y += e;
        DrawPlane(pos, size, door_color);
    }

    // west
    pos = {center.x - 0.5f, e, center.y};
    size = {w, h};

    if (tile->has_solid_wall(Direction::WEST)) {
        DrawPlane(pos, size, solid_color);
    } else if (tile->has_door_wall(Direction::WEST)) {
        pos.y += e;
        DrawPlane(pos, size, door_color);
    }

    // east
    pos = {center.x + 0.5f, e, center.y};
    size = {w, h};

    if (tile->has_solid_wall(Direction::EAST)) {
        DrawPlane(pos, size, solid_color);
    } else if (tile->has_door_wall(Direction::EAST)) {
        pos.y += e;
        DrawPlane(pos, size, door_color);
    }
}

void draw_room_perimiter(int room_id, Color solid_color, Color door_color) {
    for (auto tile : world::get_room_tiles(room_id)) {
        draw_tile_perimiter(tile, solid_color, door_color);
    }
}

void draw_room_perimiter(int room_id, Color color) {
    draw_room_perimiter(room_id, color, color);
}

}  // namespace soft_tissues::editor::utils
