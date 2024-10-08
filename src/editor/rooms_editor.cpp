#include "../resources.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace soft_tissues::editor::rooms_editor {

static int ROOM_ID = -1;
static std::vector<tile::Tile *> GHOST_TILES;
static tile::TileMaterials MATERIALS;

static void select_room(int id) {
    ROOM_ID = id;
    auto tiles = world::get_room_tiles(id);

    // NOTE: Assume that the room materials is the first tile materials
    if (tiles.size() > 0) MATERIALS = tiles[0]->materials;
}

static void draw_tile_ghost(tile::Tile *tile, Color color) {
    auto mesh = resources::get_mesh("plane");
    auto material = resources::get_material_color(color);
    auto matrix = tile->get_floor_matrix();

    DrawMesh(mesh, material, matrix);
}

static void draw_tile_ghost(tile::Tile *tile, bool is_remove) {
    int room_id = world::get_tile_room_id(tile);
    bool can_remove = is_remove && room_id == ROOM_ID;
    bool can_place = !is_remove && room_id == -1;

    Color color = GRAY;
    if (can_remove || can_place) {
        color = is_remove ? RED : GREEN;
    }

    draw_tile_ghost(tile, color);
}

void update_and_draw() {
    static bool is_loaded = false;

    if (!is_loaded) {
        MATERIALS = tile::TileMaterials("brick_wall");
        is_loaded = true;
    }

    if (IsKeyPressed(KEY_ESCAPE) || world::get_rooms_count() == 0) {
        ROOM_ID = -1;
    }

    // ---------------------------------------------------------------
    bool is_remove_down = IsKeyDown(KEY_R);

    if (gui::button("[N]ew Room") || IsKeyPressed(KEY_N)) {
        ROOM_ID = world::add_room();
    }
    ImGui::SameLine();

    if (ROOM_ID != -1) {
        auto color = is_remove_down ? ImVec4(1.0, 0.0, 0.0, 1.0)
                                    : ImVec4(0.5, 0.5, 0.5, 1.0);
        ImGui::TextColored(color, "[R]emove");
    }

    ImGui::Separator();

    // ---------------------------------------------------------------
    Vector2 tile_at_cursor_pos;
    tile::Tile *tile_at_cursor = world::get_tile_at_cursor(&tile_at_cursor_pos);

    if (ROOM_ID != -1) {
        ImGui::BeginTabBar("Materials");

        if (ImGui::BeginTabItem("floor")) {
            gui::tile_material_picker(&MATERIALS.floor_key, &MATERIALS);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("wall")) {
            gui::tile_material_picker(&MATERIALS.wall_key, &MATERIALS);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ceil")) {
            gui::tile_material_picker(&MATERIALS.ceil_key, &MATERIALS);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        ImGui::Separator();

        static tile::Tile *start_tile = NULL;
        static tile::Tile *end_tile = NULL;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            start_tile = tile_at_cursor;
            end_tile = tile_at_cursor;
        } else if (start_tile != NULL && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (tile_at_cursor != NULL) end_tile = tile_at_cursor;

            GHOST_TILES = world::get_tiles_between_corners(start_tile, end_tile);
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            for (auto tile : GHOST_TILES) {
                int room_id = world::get_tile_room_id(tile);

                if (room_id == ROOM_ID && is_remove_down) {
                    world::clear_tile(tile);
                } else if (room_id == -1 && !is_remove_down) {
                    world::add_tile_to_room(tile, ROOM_ID);
                }
            }

            GHOST_TILES.clear();
            start_tile = NULL;
            end_tile = NULL;
        }

        world::set_room_tile_materials(ROOM_ID, MATERIALS);
        utils::draw_room_perimiter(ROOM_ID, GREEN, ORANGE);

        for (auto tile : GHOST_TILES) {
            draw_tile_ghost(tile, is_remove_down);
        }

        if (tile_at_cursor) {
            draw_tile_ghost(tile_at_cursor, is_remove_down);

            tile::Tile *nb = world::get_nearest_tile_neighbor_at_position(
                tile_at_cursor_pos
            );

            if (nb && !is_remove_down) {
                int room_id_0 = world::get_tile_room_id(tile_at_cursor);
                int room_id_1 = world::get_tile_room_id(nb);

                if (room_id_0 != -1 && room_id_1 != -1 && room_id_0 != room_id_1
                    && (room_id_0 == ROOM_ID || room_id_1 == ROOM_ID)) {
                    draw_tile_ghost(tile_at_cursor, ORANGE);
                    draw_tile_ghost(nb, ORANGE);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        world::set_door_between_neighbor_tiles(tile_at_cursor, nb);
                    }
                }
            }
        }
    } else {
        int room_id = world::get_tile_room_id(tile_at_cursor);
        utils::draw_room_perimiter(room_id, YELLOW);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            select_room(room_id);
        }
    }
}

}  // namespace soft_tissues::editor::rooms_editor
