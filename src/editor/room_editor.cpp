#include "../resources.hpp"
#include "../tile.hpp"
#include "../world.hpp"
#include "editor.hpp"
#include "imgui/imgui.h"
#include "raylib/raylib.h"
#include <cstdio>
#include <string>

namespace soft_tissues::editor::room_editor {

using namespace utils;

static int ROOM_ID = -1;
static std::vector<tile::Tile *> GHOST_TILES;
static tile::TileMaterials MATERIALS;

static void select_room(int id) {
    ROOM_ID = id;
    auto tiles = world::get_room_tiles(id);

    // NOTE: Assume that the room materials is the first tile materials
    if (tiles.size() > 0) MATERIALS = tiles[0]->materials;
}

static void update_material_selector(pbr::MaterialPBR *material) {
    auto name = material->get_name();

    if (ImGui::BeginMenu(name.c_str())) {
        if (gui::button("Apply to all")) {
            MATERIALS = pbr::MaterialPBR(*material);
        }
        ImGui::Separator();

        for (auto &another_material : resources::MATERIALS_PBR) {
            auto another_name = another_material.get_name();
            bool is_selected = name == another_name;

            if (ImGui::MenuItem(another_name.c_str(), NULL, is_selected)) {
                *material = another_material;
            }
            gui::image(another_material.get_texture(), 30.0);
            ImGui::Separator();
        }
        ImGui::EndMenu();
    }

    gui::image(material->get_texture(), 150.0);
}

void draw_tile_ghost(tile::Tile *tile, Color color) {
    Material material = resources::get_color_material(color);
    Matrix matrix = tile->get_floor_matrix();
    DrawMesh(resources::PLANE_MESH, material, matrix);
}

void draw_tile_perimiter(tile::Tile *tile, Color color) {
    static float e = 1e-2;
    static float w = 0.2;
    static float h = w + 1.0;

    Vector2 center = world::get_tile_center(tile);

    if (tile->has_flags(tile::TILE_NORTH_WALL)) {
        DrawPlane({center.x, e, center.y - 0.5f}, {h, w}, color);
    }

    if (tile->has_flags(tile::TILE_SOUTH_WALL)) {
        DrawPlane({center.x, e, center.y + 0.5f}, {h, w}, color);
    }

    if (tile->has_flags(tile::TILE_WEST_WALL)) {
        DrawPlane({center.x - 0.5f, e, center.y}, {w, h}, color);
    }

    if (tile->has_flags(tile::TILE_EAST_WALL)) {
        DrawPlane({center.x + 0.5f, e, center.y}, {w, h}, color);
    }
}

void update_and_draw() {
    static bool is_loaded = false;
    if (!is_loaded) {
        MATERIALS = tile::TileMaterials(resources::MATERIALS_PBR[0]);
        is_loaded = true;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        ROOM_ID = -1;
    }

    if (gui::button("New Room")) {
        ROOM_ID = world::add_room();
    }
    ImGui::Separator();

    // ---------------------------------------------------------------

    // ---------------------------------------------------------------
    tile::Tile *tile_at_cursor = world::get_tile_at_cursor();

    if (ROOM_ID != -1) {
        ImGui::BeginTabBar("Materials");

        if (ImGui::BeginTabItem("floor")) {
            update_material_selector(&MATERIALS.floor);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("wall")) {
            update_material_selector(&MATERIALS.wall);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("ceil")) {
            update_material_selector(&MATERIALS.ceil);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        ImGui::Separator();

        static tile::Tile *start_tile = NULL;
        static tile::Tile *end_tile = NULL;
        bool is_remove = IsKeyDown(KEY_LEFT_CONTROL);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            start_tile = tile_at_cursor;
            end_tile = tile_at_cursor;
        } else if (start_tile != NULL && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (tile_at_cursor != NULL) end_tile = tile_at_cursor;

            GHOST_TILES = world::get_tiles_between_corners(start_tile, end_tile);
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            for (auto tile : GHOST_TILES) {
                int room_id = world::get_tile_room_id(tile);

                if (room_id == ROOM_ID && is_remove) {
                    world::clear_tile(tile);
                } else if (room_id == -1 && !is_remove) {
                    world::add_tile_to_room(tile, ROOM_ID);
                }
            }

            GHOST_TILES.clear();
            start_tile = NULL;
            end_tile = NULL;
        }

        for (auto tile : world::get_room_tiles(ROOM_ID)) {
            tile->materials = MATERIALS;
            draw_tile_perimiter(tile, GREEN);
        }

        Color ghost_color = is_remove ? RED : GREEN;

        for (auto tile : GHOST_TILES) {
            draw_tile_ghost(tile, ghost_color);
        }

        if (tile_at_cursor != NULL) {
            draw_tile_ghost(tile_at_cursor, ghost_color);
        }
    } else {
        int room_id = world::get_tile_room_id(tile_at_cursor);
        for (auto tile : world::get_room_tiles(room_id)) {
            draw_tile_perimiter(tile, YELLOW);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            select_room(room_id);
        }
    }
}

}  // namespace soft_tissues::editor::room_editor
