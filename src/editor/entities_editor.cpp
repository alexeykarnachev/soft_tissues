#include "../component/component.hpp"
#include "../globals.hpp"
#include "../tile.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
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

static void reset() {
    STATE = State::NONE;
    ROOM_ID = -1;
    gizmo::detach();
}

static void update_and_draw_transformation(transform::Transform *tr) {
    ImGui::SeparatorText("Transformation");

    {
        static float speed = 0.1;
        float *v = (float *)&tr->position;

        ImGui::PushID(&tr->position);
        ImGui::DragFloat3("Position", v, speed);
        ImGui::PopID();
    }

    {
        static float speed = PI / 16.0;
        static float min = -2.0 * PI;
        static float max = 2.0 * PI;
        float *v = (float *)&tr->rotation;

        ImGui::PushID(&tr->rotation);
        ImGui::DragFloat3("Rotation", v, speed, min, max);
        ImGui::PopID();
    }
}

void update_and_draw() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        reset();
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
            auto &tr = globals::registry.get<component::Transform>(player);

            update_and_draw_transformation(&tr);

            gizmo::attach(player);
        } break;
        default: {
            assert(ROOM_ID == -1);
        } break;
    }
}

}  // namespace soft_tissues::editor::entities_editor
