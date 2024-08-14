#include "editor.hpp"

#include "../camera.hpp"
#include "../component/component.hpp"
#include "../globals.hpp"
#include "../resources.hpp"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <functional>

namespace soft_tissues::editor {

entt::entity HOVERED_ENTITY = entt::null;
bool IS_GUI_INTERACTED = false;

static const int PICKING_FBO_SIZE = 1024;
static unsigned int PICKING_FBO;
static unsigned int PICKING_TEXTURE;

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

static std::array<Tab, 2> TABS = {
    Tab("[1]Rooms", KEY_ONE, rooms_editor::update_and_draw),
    Tab("[2]Entities", KEY_TWO, entities_editor::update_and_draw),
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

    gizmo::load();

    // -------------------------------------------------------------------
    // load picking fbo
    // TODO: Factor out picking fbo creation (the same is in gizmo.cpp)
    PICKING_FBO = rlLoadFramebuffer();
    if (!PICKING_FBO) {
        TraceLog(LOG_ERROR, "RAYGIZMO: Failed to create picking fbo");
        exit(1);
    }
    rlEnableFramebuffer(PICKING_FBO);

    PICKING_TEXTURE = rlLoadTexture(
        NULL, PICKING_FBO_SIZE, PICKING_FBO_SIZE, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1
    );
    rlActiveDrawBuffers(1);
    rlFramebufferAttach(
        PICKING_FBO,
        PICKING_TEXTURE,
        RL_ATTACHMENT_COLOR_CHANNEL0,
        RL_ATTACHMENT_TEXTURE2D,
        0
    );
    if (!rlFramebufferComplete(PICKING_FBO)) {
        TraceLog(LOG_ERROR, "EDITOR: Picking fbo is not complete");
        exit(1);
    }
}

void unload() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    gizmo::unload();

    // -------------------------------------------------------------------
    // unload picking fbo
    rlUnloadFramebuffer(PICKING_FBO);
    rlUnloadTexture(PICKING_TEXTURE);
}

static void update_and_draw_tabs() {
    static std::string active_tab_name = TABS[0].name;

    ImGui::BeginTabBar("Tabs");

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

static void update_and_draw_debug() {
    // -------------------------------------------------------------------
    // graphics
    ImGui::SeparatorText("Graphics");
    ImGui::Checkbox("is_light_enabled", &globals::GRAPHICS_OPTIONS.is_light_enabled);

    // -------------------------------------------------------------------
    // flashlight
    ImGui::SeparatorText("Flashlight");
    auto flashlight = globals::registry.view<component::Flashlight>().front();
    auto &light = globals::registry.get<component::Light>(flashlight);

    gui::common_light_params(&light);
    gui::spot_light_params(&light);
}

void update_and_draw() {
    auto io = ImGui::GetIO();
    IS_GUI_INTERACTED = io.WantCaptureMouse;

    BeginMode3D(camera::CAMERA);
    begin();

    ImGui::Begin("Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    {
        update_and_draw_tabs();
        update_and_draw_debug();
    }
    ImGui::End();

    end();
    EndMode3D();

    gizmo::update_and_draw();
}

void update_hovered_entity() {
    static std::vector<entt::entity> entities;
    entities.clear();
    HOVERED_ENTITY = entt::null;

    unsigned char id = 1;
    float screen_width = GetScreenWidth();
    float screen_height = GetScreenHeight();

    rlEnableFramebuffer(PICKING_FBO);
    rlViewport(0, 0, PICKING_FBO_SIZE, PICKING_FBO_SIZE);
    rlClearColor(0, 0, 0, 0);
    rlClearScreenBuffers();
    rlEnableDepthTest();
    rlDisableColorBlend();
    BeginMode3D(camera::CAMERA);
    {
        // meshes
        auto meshes = globals::registry.view<component::MyMesh>();
        for (auto entity : meshes) {
            auto tr = globals::registry.get<component::Transform>(entity);
            auto mesh = globals::registry.get<component::MyMesh>(entity);

            Matrix matrix = tr.get_matrix();
            Material material = resources::DEFAULT_MATERIAL;
            material.maps[0].color = {id, 0, 0, 255};
            DrawMesh(mesh.mesh, material, matrix);

            entities.push_back(entity);
            // TODO: Check for overflow
            id += 1;
        }

        // light shells
        auto lights = globals::registry.view<component::Light>();
        for (auto entity : lights) {
            auto tr = globals::registry.get<component::Transform>(entity);
            Color color = {id, 0, 0, 255};
            DrawSphere(tr.get_position(), 0.2, color);

            entities.push_back(entity);
            // TODO: Check for overflow
            id += 1;
        }
    }
    EndMode3D();
    rlDisableFramebuffer();
    rlEnableColorBlend();
    rlViewport(0, 0, screen_width, screen_height);

    // -------------------------------------------------------------------
    // Pick the pixel under the mouse cursor
    // TODO: Factor out pixel picking (the same is in gizmo.cpp)
    Vector2 mouse_position = GetMousePosition();
    unsigned char *pixels = (unsigned char *)rlReadTexturePixels(
        PICKING_TEXTURE,
        PICKING_FBO_SIZE,
        PICKING_FBO_SIZE,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float x_fract = Clamp(mouse_position.x / screen_width, 0.0, 1.0);
    float y_fract = Clamp(1.0 - (mouse_position.y / screen_height), 0.0, 1.0);
    int x = (int)(PICKING_FBO_SIZE * x_fract);
    int y = (int)(PICKING_FBO_SIZE * y_fract);

    // NOTE: Currently I pick only r-component (256 entities max)
    int byte_loc = 4 * (y * PICKING_FBO_SIZE + x);
    unsigned char picked_id = pixels[byte_loc];

    free(pixels);

    if (picked_id != 0) {
        int picked_idx = picked_id - 1;
        HOVERED_ENTITY = entities[picked_idx];
    }
}

}  // namespace soft_tissues::editor
