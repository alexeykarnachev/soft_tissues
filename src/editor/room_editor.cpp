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

static void update_material(pbr::MaterialPBR *material) {
    auto name = material->get_name();

    if (ImGui::BeginMenu(name.c_str())) {
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

void update_and_draw() {
    static bool is_loaded = false;
    if (!is_loaded) {
        MATERIALS = tile::TileMaterials(resources::MATERIALS_PBR[0]);
        is_loaded = true;
    }

    if (gui::button("New Room")) {
        ROOM_ID = world::add_room();
    }
    ImGui::Separator();

    for (auto id : world::get_room_ids()) {
        auto name = "Edit room #" + std::to_string(id);

        if (id != ROOM_ID && gui::button(name.c_str())) {
            ROOM_ID = id;
            auto tiles = world::get_room_tiles(id);
            // NOTE: Assume that the room materials is the first tile materials
            if (tiles.size() > 0) MATERIALS = tiles[0]->materials;
        }

        if (id == ROOM_ID) {
            // ---------------------------------------------------------------
            ImGui::BeginTabBar("Materials");

            if (ImGui::BeginTabItem("floor")) {
                update_material(&MATERIALS.floor);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("wall")) {
                update_material(&MATERIALS.wall);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("ceil")) {
                update_material(&MATERIALS.ceil);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();

            // ---------------------------------------------------------------
            static tile::Tile *start_tile = NULL;
            static tile::Tile *end_tile = NULL;

            tile::Tile *tile = world::get_tile_at_cursor();

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                start_tile = tile;
                end_tile = tile;
            } else if (start_tile != NULL && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                if (tile != NULL) end_tile = tile;

                GHOST_TILES = world::get_tiles_between_corners(start_tile, end_tile);
            } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                for (auto tile : GHOST_TILES) {
                    if (tile->is_empty()) world::add_tile_to_room(tile, ROOM_ID);
                }

                GHOST_TILES.clear();
                start_tile = NULL;
                end_tile = NULL;
            }

            // ---------------------------------------------------------------
            for (auto tile : world::get_room_tiles(ROOM_ID)) {
                tile->materials = MATERIALS;
            }

            for (auto tile : GHOST_TILES) {
                draw_tile_ghost(tile, WHITE);
            }

            if (tile != NULL) {
                draw_tile_ghost(tile, GREEN);
            }
        }

        ImGui::Separator();
    }
}

}  // namespace soft_tissues::editor::room_editor
