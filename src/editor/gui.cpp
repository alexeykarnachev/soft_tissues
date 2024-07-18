#include "editor.hpp"
#include "imgui/imgui.h"
#include <cmath>
#include <cstdio>

namespace soft_tissues::editor::gui {

static const ImVec4 COLOR_CANCEL = {0.8, 0.2, 0.1, 1.0};
static const ImVec4 COLOR_ACCEPT = {0.2, 0.8, 0.1, 1.0};

static void push_id() {
    static int id = 0;
    static int prev_frame = 0;
    int curr_frame = ImGui::GetFrameCount();

    if (curr_frame != prev_frame) {
        id = 0;
        prev_frame = curr_frame;
    }

    ImGui::PushID(id++);
}

static void pop_id() {
    ImGui::PopID();
}

bool collapsing_header(const char *name, bool is_opened) {
    int flags = is_opened ? ImGuiTreeNodeFlags_DefaultOpen : 0;
    return ImGui::CollapsingHeader(name, flags);
}

bool collapsing_header(const char *name) {
    return collapsing_header(name, true);
}

bool button(const char *name, bool is_enabled) {
    push_id();
    if (!is_enabled) ImGui::BeginDisabled();
    bool is_clicked = ImGui::Button(name);
    if (!is_enabled) ImGui::EndDisabled();
    pop_id();

    return is_clicked;
}

bool button_color(const char *name, ImVec4 color, bool is_enabled) {
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    bool is_clicked = button(name, is_enabled);
    ImGui::PopStyleColor();

    return is_clicked;
}

bool button_cancel(bool is_enabled) {
    return button_color("Cancel", COLOR_CANCEL, is_enabled);
}

bool button_accept(bool is_enabled) {
    return button_color("Accept", COLOR_ACCEPT, is_enabled);
}

}  // namespace soft_tissues::editor::gui
