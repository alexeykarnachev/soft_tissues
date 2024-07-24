#include "editor.hpp"

#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"
#include <array>
#include <cmath>
#include <functional>

namespace soft_tissues::editor {

class Tab {
public:
    std::string name;
    int key;
    std::function<void()> fn;

    Tab(std::string name, int key, std::function<void()> fn)
        : name(name)
        , key(key)
        , fn(fn) {}
};

static void pass() {}

static std::array<Tab, 2> TABS = {
    Tab("[1]Room", KEY_ONE, room_editor::update_and_draw),
    Tab("[2]TODO", KEY_TWO, pass),
};

static void begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

static void end() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void load() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    GLFWwindow *window = (GLFWwindow *)GetWindowHandle();
    glfwGetWindowUserPointer(window);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    ImGui::StyleColorsDark();
}

void unload() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void update_and_draw() {
    begin();

    static std::string active_tab_name = TABS[0].name;

    ImGui::Begin("Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    {
        ImGui::BeginTabBar("States");

        for (const auto &tab : TABS) {
            if (IsKeyPressed(tab.key)) active_tab_name = tab.name;
        }

        for (const auto &tab : TABS) {
            bool is_open = (active_tab_name == tab.name);
            int flag = is_open ? ImGuiTabItemFlags_SetSelected : 0;
            if (ImGui::BeginTabItem(tab.name.c_str(), NULL, flag)) {
                if (ImGui::IsItemActive()) active_tab_name = tab.name;
                tab.fn();
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
    ImGui::End();

    end();
}

}  // namespace soft_tissues::editor
