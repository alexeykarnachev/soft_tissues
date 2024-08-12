#include "../camera.hpp"
#include "../component/component.hpp"
#include "../globals.hpp"
#include "editor.hpp"
#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include <stdlib.h>
#include <string.h>
#include <utility>

namespace soft_tissues::editor::gizmo {

static constexpr int PICKING_FBO_SIZE = 512;

static constexpr Vector3 X_AXIS = {1.0, 0.0, 0.0};
static constexpr Vector3 Y_AXIS = {0.0, 1.0, 0.0};
static constexpr Vector3 Z_AXIS = {0.0, 0.0, 1.0};

static constexpr float SIZE = 0.12f;
static constexpr float HANDLE_DRAW_THICKNESS = 5.0f;
static constexpr float ACTIVE_AXIS_DRAW_THICKNESS = 2.0f;
static constexpr float AXIS_HANDLE_LENGTH = 1.2f;
static constexpr float AXIS_HANDLE_TIP_LENGTH = 0.3f;
static constexpr float AXIS_HANDLE_TIP_RADIUS = 0.1f;
static constexpr float PLANE_HANDLE_OFFSET = 0.4f;
static constexpr float PLANE_HANDLE_SIZE = 0.2f;

static const std::string SHADER_VERT = R"vert(
#version 330
in vec3 vertexPosition;
in vec4 vertexColor;
out vec4 fragColor;
out vec3 fragPosition;
uniform mat4 mvp;
void main() {
    fragColor = vertexColor;
    fragPosition = vertexPosition;
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)vert";

static const std::string SHADER_FRAG = R"frag(
#version 330
in vec4 fragColor;
in vec3 fragPosition;
uniform vec3 cameraPosition;
uniform vec3 gizmoPosition;
out vec4 finalColor;
void main() {
    vec3 r = normalize(fragPosition - gizmoPosition);
    vec3 c = normalize(fragPosition - cameraPosition);
    if (dot(r, c) > 0.1) discard;
    finalColor = fragColor;
}
)frag";

struct Update {
    Vector3 translation;
    Vector3 axis;
    float angle;
};

enum HandleId {
    HANDLE_X,

    ROT_HANDLE_X,
    AXIS_HANDLE_X,
    PLANE_HANDLE_X,

    HANDLE_Y,

    ROT_HANDLE_Y,
    AXIS_HANDLE_Y,
    PLANE_HANDLE_Y,

    HANDLE_Z,

    ROT_HANDLE_Z,
    AXIS_HANDLE_Z,
    PLANE_HANDLE_Z
};

struct Handle {
    Vector3 position;
    Vector3 axis;
    Color color;
    float distToCamera;
};

struct XYZColors {
    Color x;
    Color y;
    Color z;
};

struct HandleColors {
    XYZColors rot;
    XYZColors axis;
    XYZColors plane;
};

struct Handles {
    Handle arr[3];
};

static Shader SHADER;
static int SHADER_CAMERA_POSITION_LOC;
static int SHADER_GIZMO_POSITION_LOC;

static unsigned int PICKING_FBO;
static unsigned int PICKING_TEXTURE;

static Update UPDATE;
static bool IS_LOADED = false;
static entt::entity ENTITY = entt::null;

State STATE = State::COLD;

static Handles sort_handles(Handle h0, Handle h1, Handle h2) {
    if (h0.distToCamera < h1.distToCamera) std::swap(h0, h1);
    if (h1.distToCamera < h2.distToCamera) std::swap(h1, h2);
    if (h0.distToCamera < h1.distToCamera) std::swap(h0, h1);

    Handles handles = {.arr = {h0, h1, h2}};
    return handles;
}

static XYZColors get_xyz_colors(Vector3 current_axis, bool is_hot) {
    Color x = is_hot && current_axis.x == 1.0f ? WHITE : RED;
    Color y = is_hot && current_axis.y == 1.0f ? WHITE : GREEN;
    Color z = is_hot && current_axis.z == 1.0f ? WHITE : BLUE;
    XYZColors colors = {x, y, z};
    return colors;
}

static void draw(HandleColors colors) {
    auto tr = globals::registry.get<component::Transform>(ENTITY);
    Camera3D camera = camera::CAMERA;

    Vector3 position = tr.get_position();
    float radius = SIZE * Vector3Distance(camera.position, position);

    BeginMode3D(camera);
    rlSetLineWidth(HANDLE_DRAW_THICKNESS);
    rlDisableDepthTest();

    // ---------------------------------------------------------------
    // Draw plane handles
    {
        float offset = radius * PLANE_HANDLE_OFFSET;
        float size = radius * PLANE_HANDLE_SIZE;

        Vector3 px = Vector3Add(position, {0.0f, offset, offset});
        Vector3 py = Vector3Add(position, {offset, 0.0f, offset});
        Vector3 pz = Vector3Add(position, {offset, offset, 0.0f});

        Handle hx = {px, Z_AXIS, colors.plane.x, Vector3DistanceSqr(px, camera.position)};
        Handle hy = {py, Y_AXIS, colors.plane.y, Vector3DistanceSqr(py, camera.position)};
        Handle hz = {pz, X_AXIS, colors.plane.z, Vector3DistanceSqr(pz, camera.position)};
        Handles handles = sort_handles(hx, hy, hz);

        rlDisableBackfaceCulling();
        for (int i = 0; i < 3; ++i) {
            Handle *h = &handles.arr[i];
            rlPushMatrix();
            rlTranslatef(h->position.x, h->position.y, h->position.z);
            rlRotatef(90.0f, h->axis.x, h->axis.y, h->axis.z);
            DrawPlane(Vector3Zero(), Vector2Scale(Vector2One(), size), h->color);
            rlPopMatrix();
        }
    }

    // ---------------------------------------------------------------
    // Draw rotation handles
    {
        BeginShaderMode(SHADER);
        SetShaderValue(
            SHADER, SHADER_CAMERA_POSITION_LOC, &camera.position, SHADER_UNIFORM_VEC3
        );
        SetShaderValue(SHADER, SHADER_GIZMO_POSITION_LOC, &position, SHADER_UNIFORM_VEC3);
        DrawCircle3D(position, radius, Y_AXIS, 90.0f, colors.rot.x);
        DrawCircle3D(position, radius, X_AXIS, 90.0f, colors.rot.y);
        DrawCircle3D(position, radius, X_AXIS, 0.0f, colors.rot.z);
        EndShaderMode();
    }

    // ---------------------------------------------------------------
    // Draw axis handles
    {
        float length = radius * AXIS_HANDLE_LENGTH;
        float tip_length = radius * AXIS_HANDLE_TIP_LENGTH;
        float tip_radius = radius * AXIS_HANDLE_TIP_RADIUS;

        Vector3 px = Vector3Add(position, Vector3Scale(X_AXIS, length));
        Vector3 py = Vector3Add(position, Vector3Scale(Y_AXIS, length));
        Vector3 pz = Vector3Add(position, Vector3Scale(Z_AXIS, length));

        Handle hx = {px, X_AXIS, colors.axis.x, Vector3DistanceSqr(px, camera.position)};
        Handle hy = {py, Y_AXIS, colors.axis.y, Vector3DistanceSqr(py, camera.position)};
        Handle hz = {pz, Z_AXIS, colors.axis.z, Vector3DistanceSqr(pz, camera.position)};
        Handles handles = sort_handles(hx, hy, hz);

        for (int i = 0; i < 3; ++i) {
            Handle *h = &handles.arr[i];
            Vector3 tip_end = Vector3Add(h->position, Vector3Scale(h->axis, tip_length));
            DrawLine3D(position, h->position, h->color);
            DrawCylinderEx(h->position, tip_end, tip_radius, 0.0f, 16, h->color);
        }
    }
    EndMode3D();

    // ---------------------------------------------------------------
    // Draw long white line which represents current active axis
    if (STATE == ACTIVE_ROT || STATE == ACTIVE_AXIS) {
        BeginMode3D(camera);
        rlSetLineWidth(ACTIVE_AXIS_DRAW_THICKNESS);
        Vector3 halfAxisLine = Vector3Scale(UPDATE.axis, 1000.0f);
        DrawLine3D(
            Vector3Subtract(position, halfAxisLine),
            Vector3Add(position, halfAxisLine),
            WHITE
        );
        EndMode3D();
    }

    // ---------------------------------------------------------------
    // Draw white line from the gizmo's center to the mouse cursor when rotating
    if (STATE == ACTIVE_ROT) {
        rlSetLineWidth(ACTIVE_AXIS_DRAW_THICKNESS);
        DrawLineV(GetWorldToScreen(position, camera), GetMousePosition(), WHITE);
    }
}

void unload(void) {
    if (!IS_LOADED) {
        TraceLog(LOG_WARNING, "RAYGIZMO: Gizmo is already unloaded, skip");
        return;
    }

    UnloadShader(SHADER);
    rlUnloadFramebuffer(PICKING_FBO);
    rlUnloadTexture(PICKING_TEXTURE);

    IS_LOADED = false;
    TraceLog(LOG_INFO, "RAYGIZMO: Gizmo unloaded");
}

void attach(entt::entity entity) {
    ENTITY = entity;
}

void detach() {
    ENTITY = entt::null;
}

void load() {
    if (IS_LOADED) {
        TraceLog(LOG_WARNING, "RAYGIZMO: Gizmo is already loaded, skip");
        return;
    }

    // -------------------------------------------------------------------
    // Load shader
    SHADER = LoadShaderFromMemory(SHADER_VERT.c_str(), SHADER_FRAG.c_str());
    SHADER_CAMERA_POSITION_LOC = GetShaderLocation(SHADER, "cameraPosition");
    SHADER_GIZMO_POSITION_LOC = GetShaderLocation(SHADER, "gizmoPosition");

    // -------------------------------------------------------------------
    // Load picking fbo
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
        TraceLog(LOG_ERROR, "RAYGIZMO: Picking fbo is not complete");
        exit(1);
    }

    TraceLog(LOG_INFO, "RAYGIZMO: Gizmo loaded");
    IS_LOADED = true;
}

static void update() {
    auto &tr = globals::registry.get<component::Transform>(ENTITY);

    // -------------------------------------------------------------------
    // Draw gizmo into the picking fbo for the mouse pixel-picking
    rlEnableFramebuffer(PICKING_FBO);
    rlViewport(0, 0, PICKING_FBO_SIZE, PICKING_FBO_SIZE);
    rlClearColor(0, 0, 0, 0);
    rlClearScreenBuffers();
    rlDisableColorBlend();

    HandleColors colors = {
        {
            {ROT_HANDLE_X, 0, 0, 0},
            {ROT_HANDLE_Y, 0, 0, 0},
            {ROT_HANDLE_Z, 0, 0, 0},
        },
        {
            {AXIS_HANDLE_X, 0, 0, 0},
            {AXIS_HANDLE_Y, 0, 0, 0},
            {AXIS_HANDLE_Z, 0, 0, 0},
        },
        {
            {PLANE_HANDLE_X, 0, 0, 0},
            {PLANE_HANDLE_Y, 0, 0, 0},
            {PLANE_HANDLE_Z, 0, 0, 0},
        }
    };

    draw(colors);

    rlDisableFramebuffer();
    rlEnableColorBlend();
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());

    // -------------------------------------------------------------------
    // Pick the pixel under the mouse cursor
    Vector2 mouse_position = GetMousePosition();
    unsigned char *pixels = (unsigned char *)rlReadTexturePixels(
        PICKING_TEXTURE,
        PICKING_FBO_SIZE,
        PICKING_FBO_SIZE,
        RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    );

    float x_fract = Clamp(mouse_position.x / (float)GetScreenWidth(), 0.0, 1.0);
    float y_fract = Clamp(1.0 - (mouse_position.y / (float)GetScreenHeight()), 0.0, 1.0);
    int x = (int)(PICKING_FBO_SIZE * x_fract);
    int y = (int)(PICKING_FBO_SIZE * y_fract);
    int idx = 4 * (y * PICKING_FBO_SIZE + x);
    unsigned char picked_id = pixels[idx];

    free(pixels);

    // -------------------------------------------------------------------
    // Update gizmo
    UPDATE.angle = 0.0;
    UPDATE.translation = Vector3Zero();

    bool is_lmb_down = IsMouseButtonDown(0);
    if (!is_lmb_down) STATE = COLD;

    if (STATE < ACTIVE) {
        if (picked_id < HANDLE_Y) UPDATE.axis = X_AXIS;
        else if (picked_id < HANDLE_Z) UPDATE.axis = Y_AXIS;
        else UPDATE.axis = Z_AXIS;

        if (picked_id % 4 == 1) STATE = is_lmb_down ? ACTIVE_ROT : HOT_ROT;
        else if (picked_id % 4 == 2) STATE = is_lmb_down ? ACTIVE_AXIS : HOT_AXIS;
        else if (picked_id % 4 == 3) STATE = is_lmb_down ? ACTIVE_PLANE : HOT_PLANE;
    }

    Vector2 delta = GetMouseDelta();
    bool is_mouse_moved = (fabs(delta.x) + fabs(delta.y)) > EPSILON;
    if (!is_mouse_moved) return;

    Vector3 position = tr.get_position();
    Camera3D camera = camera::CAMERA;
    switch (STATE) {
        case ACTIVE_ROT: {
            Vector2 p1 = Vector2Subtract(
                GetMousePosition(), GetWorldToScreen(position, camera)
            );
            Vector2 p0 = Vector2Subtract(p1, GetMouseDelta());

            // Get angle between two vectors:
            float angle = 0.0f;
            float dot = Vector2DotProduct(Vector2Normalize(p1), Vector2Normalize(p0));
            if (1.0 - fabs(dot) > EPSILON) {
                angle = acos(dot);
                float z = p1.x * p0.y - p1.y * p0.x;

                if (fabs(z) < EPSILON) angle = 0.0;
                else if (z <= 0) angle *= -1.0;
            }

            // If we look at the gizmo from behind, we should flip the rotation
            if (Vector3DotProduct(UPDATE.axis, position)
                > Vector3DotProduct(UPDATE.axis, camera.position)) {
                angle *= -1;
            }

            UPDATE.angle = angle;
        } break;
        case ACTIVE_AXIS: {
            Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
            Ray r = GetMouseRay(p, camera);

            // Get two lines nearest point
            Vector3 line0point0 = camera.position;
            Vector3 line0point1 = Vector3Add(line0point0, r.direction);
            Vector3 line1point0 = position;
            Vector3 line1point1 = Vector3Add(line1point0, UPDATE.axis);
            Vector3 vec0 = Vector3Subtract(line0point1, line0point0);
            Vector3 vec1 = Vector3Subtract(line1point1, line1point0);
            Vector3 plane_vec = Vector3Normalize(Vector3CrossProduct(vec0, vec1));
            Vector3 plane_normal = Vector3Normalize(Vector3CrossProduct(vec0, plane_vec));

            // Intersect line and plane
            float dot = Vector3DotProduct(plane_normal, vec1);
            if (fabs(dot) > EPSILON) {
                Vector3 w = Vector3Subtract(line1point0, line0point0);
                float k = -Vector3DotProduct(plane_normal, w) / dot;
                Vector3 isect = Vector3Add(line1point0, Vector3Scale(vec1, k));
                UPDATE.translation = Vector3Subtract(isect, position);
            }
        } break;
        case ACTIVE_PLANE: {
            Vector2 p = Vector2Add(GetWorldToScreen(position, camera), GetMouseDelta());
            Ray r = GetMouseRay(p, camera);

            // Collide ray and plane
            float denominator = r.direction.x * UPDATE.axis.x
                                + r.direction.y * UPDATE.axis.y
                                + r.direction.z * UPDATE.axis.z;

            if (fabs(denominator) > EPSILON) {
                float t = ((position.x - r.position.x) * UPDATE.axis.x
                           + (position.y - r.position.y) * UPDATE.axis.y
                           + (position.z - r.position.z) * UPDATE.axis.z)
                          / denominator;

                if (t > 0) {
                    Vector3 c = Vector3Add(r.position, Vector3Scale(r.direction, t));
                    UPDATE.translation = Vector3Subtract(c, position);
                }
            }
        } break;
        default: break;
    }

    // -------------------------------------------------------------------
    // update entity
    if (Vector3Length(UPDATE.translation) > EPSILON) {
        tr._position = Vector3Add(tr._position, UPDATE.translation);
    }

    if (std::abs(UPDATE.angle) > EPSILON) {
        tr.rotate_by_axis_angle(UPDATE.axis, UPDATE.angle);
    }
}

void update_and_draw() {
    if (!IS_LOADED) {
        TraceLog(LOG_ERROR, "RAYGIZMO: Gizmo is not loaded");
        exit(1);
    }

    if (ENTITY == entt::null) return;

    update();

    HandleColors colors;
    colors.rot = get_xyz_colors(UPDATE.axis, STATE == HOT_ROT || STATE == ACTIVE_ROT);
    colors.axis = get_xyz_colors(UPDATE.axis, STATE == HOT_AXIS || STATE == ACTIVE_AXIS);
    colors.plane = get_xyz_colors(
        UPDATE.axis, STATE == HOT_PLANE || STATE == ACTIVE_PLANE
    );

    draw(colors);

    rlSetLineWidth(1.0);
    rlEnableDepthTest();
    rlEnableBackfaceCulling();
}

}  // namespace soft_tissues::editor::gizmo
