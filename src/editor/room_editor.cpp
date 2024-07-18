#include "../resources.hpp"
#include "../tile.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "raylib/raylib.h"
#include <cstdio>
#include <unordered_set>

namespace soft_tissues::editor::room_editor {

static std::unordered_set<tile::Tile *> TILES;
static tile::TileMaterials MATERIALS;

static bool can_add_tile(tile::Tile *tile) {
    if (!tile->is_empty()) return false;
    if (TILES.size() == 0) return true;

    auto neighbors = world::get_tile_neighbors(tile->get_id());
    bool has_neighbor = false;
    for (auto tile : neighbors) {
        has_neighbor |= TILES.count(tile) == 1;
    }

    if (!has_neighbor) return false;

    return true;
}

static void reset() {
    for (auto tile : TILES) {
        tile->flags = 0;
    }
}

static void add_tile(tile::Tile *tile) {
    if (!can_add_tile(tile)) return;

    tile->materials = MATERIALS;

    world::set_room_tile_flags(tile);
    auto neighbors = world::get_tile_neighbors(tile->get_id());

    for (auto nb : neighbors) {
        if (nb && !nb->is_empty()) {
            nb->materials = MATERIALS;
            world::set_room_tile_flags(nb);
        }
    }

    world::set_room_tile_flags(tile);
    TILES.insert(tile);
}

void update_and_draw() {
    static bool is_loaded = false;
    if (!is_loaded) {
        MATERIALS = tile::TileMaterials();
    }

    if (editor::STATE == editor::EditorState::NEW_ROOM_CREATION) {
        if (gui::button_cancel()) {
            STATE = EditorState::NONE;
            reset();
        }

        // auto material = NEW_ROOM.materials.floor;
        // ImGui::SeparatorText("Floor");
        // ImGui::Text(material.get_name().c_str(), "");
        // material.get_material().maps[0].texture

        // ---------------------------------------------------------------
        tile::Tile *tile = world::get_tile_at_cursor();

        // ghost tile
        if (tile) {
            Color color = can_add_tile(tile) ? GREEN : RED;
            Material material = resources::get_color_material(color);
            Matrix matrix = tile->get_floor_matrix();
            DrawMesh(resources::PLANE_MESH, material, matrix);

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                add_tile(tile);
            }
        }

    } else {
        if (gui::button("Create")) {
            STATE = EditorState::NEW_ROOM_CREATION;
        }

        ImGui::SameLine();

        gui::button_cancel(false);
    }

    // new room tiles
    for (auto tile : TILES) {
        Matrix matrix = tile->get_floor_matrix();
        Material material = resources::get_color_material(RAYWHITE);
        DrawMesh(resources::PLANE_MESH, material, matrix);
    }
}

}  // namespace soft_tissues::editor::room_editor
