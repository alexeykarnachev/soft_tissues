#include "../camera.hpp"
#include "../component/component.hpp"
#include "../globals.hpp"
#include "../tile.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include <cassert>
#include <cstdio>

namespace soft_tissues::editor::entities_editor {

enum class State {
    NONE,
    SELECTING_ROOM,
    EDITING_OBJECT,
};

static State STATE = State::NONE;
static int ROOM_ID = -1;

static Vector3 ENTITY_POSITION = Vector3Zero();

void update_and_draw() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        STATE = State::NONE;
    }

    // ---------------------------------------------------------------
    bool is_remove_down = IsKeyDown(KEY_R);

    if (gui::button("[N]ew Object") || IsKeyPressed(KEY_N)) {
        STATE = State::SELECTING_ROOM;
    }

    // ---------------------------------------------------------------
    switch (STATE) {
        case State::SELECTING_ROOM: {
            assert(ROOM_ID == -1);

            tile::Tile *tile_at_cursor = world::get_tile_at_cursor();
            int room_id = world::get_tile_room_id(tile_at_cursor);

            utils::draw_room_perimiter_walls(room_id, YELLOW);

            if (room_id != -1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                ROOM_ID = room_id;
                STATE = State::EDITING_OBJECT;
            }
        } break;
        case State::EDITING_OBJECT: {
            assert(ROOM_ID != -1);

            utils::draw_room_perimiter_walls(ROOM_ID, GREEN);

            auto player = globals::registry.view<component::Player>().front();
            gizmo::attach_to_entity(player);
        } break;
        default: {
            assert(ROOM_ID == -1);
        } break;
    }
}

}  // namespace soft_tissues::editor::entities_editor
