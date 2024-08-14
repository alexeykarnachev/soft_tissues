#include "utils.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace soft_tissues::utils {

// -----------------------------------------------------------------------
// enums
Direction flip_direction(Direction direction) {
    switch (direction) {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case WEST: return EAST;
        case EAST: return WEST;
    }

    return NORTH;
}

// -----------------------------------------------------------------------
// texture
Texture load_texture(std::string dir_path, std::string file_name) {
    Texture texture;
    auto file_path = dir_path + "/" + file_name;

    if (!std::filesystem::exists(file_path)) {
        unsigned char pixels[4] = {0, 0, 0, 0};
        texture.id = rlLoadTexture(pixels, 1, 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        TraceLog(LOG_INFO, "Texture is missing: %s", file_path.c_str());
        return texture;
    }

    texture = LoadTexture(file_path.c_str());

    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);

    return texture;
}

// -----------------------------------------------------------------------
// shader
std::string get_shader_file_path(const std::string &file_name) {
    auto file_path = "resources/shaders/" + file_name;
    return file_path;
}

std::string load_shader_src(const std::string &file_name) {
    const std::string version_src = "#version 460 core";
    std::ifstream common_file(get_shader_file_path("common.glsl"));
    std::ifstream shader_file(get_shader_file_path(file_name));

    std::stringstream common_stream, shader_stream;
    common_stream << common_file.rdbuf();
    shader_stream << shader_file.rdbuf();

    std::string common_src = common_stream.str();
    std::string shader_src = shader_stream.str();

    std::string full_src = version_src + "\n" + common_src + "\n" + shader_src;

    return full_src;
}

Shader load_shader(const std::string &vs_file_name, const std::string &fs_file_name) {
    auto vs = load_shader_src(vs_file_name);
    auto fs = load_shader_src(fs_file_name);
    Shader shader = LoadShaderFromMemory(vs.c_str(), fs.c_str());

    if (!IsShaderReady(shader)) {
        throw std::runtime_error(
            "Failed to load the shader: " + vs_file_name + ", " + fs_file_name
        );
    }

    return shader;
}

// -----------------------------------------------------------------------
// shader attributes and uniforms
int get_attribute_loc(Shader shader, std::string name, bool is_fail_allowed) {
    int loc = GetShaderLocationAttrib(shader, name.c_str());
    if (!is_fail_allowed && loc == -1) {
        throw std::runtime_error("Failed to find vertex attribute: " + name);
    }

    return loc;
}

int get_uniform_loc(Shader shader, std::string name, bool is_fail_allowed) {
    int loc = GetShaderLocation(shader, name.c_str());
    if (!is_fail_allowed && loc == -1) {
        throw std::runtime_error("Failed to find uniform: " + name);
    }

    return loc;
}

// -----------------------------------------------------------------------
// math and geometry
RayCollision get_cursor_floor_rect_collision(Rectangle rect, Camera camera) {
    Vector2 mouse_position = GetMousePosition();
    Ray ray = GetScreenToWorldRay(mouse_position, camera);

    // TODO: factor these out into RectangleExt struct
    float min_x = rect.x;
    float max_x = rect.x + rect.width;
    float min_y = rect.y;
    float max_y = rect.y + rect.height;

    Vector3 top_left = {min_x, 0.0, min_y};
    Vector3 top_right = {max_x, 0.0, min_y};
    Vector3 bot_right = {max_x, 0.0, max_y};
    Vector3 bot_left = {min_x, 0.0, max_y};

    RayCollision collision = GetRayCollisionQuad(
        ray, top_left, top_right, bot_right, bot_left
    );
    return collision;
}

// -----------------------------------------------------------------------
// mesh
void gen_mesh_tangents(Mesh *mesh) {
    if ((mesh->vertices == NULL) || (mesh->texcoords == NULL)) {
        TRACELOG(
            LOG_WARNING,
            "MESH: Tangents generation requires texcoord vertex attribute data"
        );
        return;
    }

    if (mesh->tangents == NULL)
        mesh->tangents = (float *)malloc(mesh->vertexCount * 4 * sizeof(float));
    else {
        free(mesh->tangents);
        mesh->tangents = (float *)malloc(mesh->vertexCount * 4 * sizeof(float));
    }
    memset(mesh->tangents, 0, mesh->vertexCount * 4 * sizeof(float));

    Vector3 *tan1 = (Vector3 *)malloc(mesh->vertexCount * sizeof(Vector3));
    Vector3 *tan2 = (Vector3 *)malloc(mesh->vertexCount * sizeof(Vector3));
    memset(tan1, 0, mesh->vertexCount * sizeof(Vector3));
    memset(tan2, 0, mesh->vertexCount * sizeof(Vector3));

    for (int i = 0; i < mesh->triangleCount; i++) {
        int index0 = mesh->indices[i * 3 + 0];
        int index1 = mesh->indices[i * 3 + 1];
        int index2 = mesh->indices[i * 3 + 2];

        Vector3 v0 = {
            mesh->vertices[index0 * 3 + 0],
            mesh->vertices[index0 * 3 + 1],
            mesh->vertices[index0 * 3 + 2]
        };
        Vector3 v1 = {
            mesh->vertices[index1 * 3 + 0],
            mesh->vertices[index1 * 3 + 1],
            mesh->vertices[index1 * 3 + 2]
        };
        Vector3 v2 = {
            mesh->vertices[index2 * 3 + 0],
            mesh->vertices[index2 * 3 + 1],
            mesh->vertices[index2 * 3 + 2]
        };

        Vector2 uv0 = {mesh->texcoords[index0 * 2 + 0], mesh->texcoords[index0 * 2 + 1]};
        Vector2 uv1 = {mesh->texcoords[index1 * 2 + 0], mesh->texcoords[index1 * 2 + 1]};
        Vector2 uv2 = {mesh->texcoords[index2 * 2 + 0], mesh->texcoords[index2 * 2 + 1]};

        Vector3 deltaPos1 = Vector3Subtract(v1, v0);
        Vector3 deltaPos2 = Vector3Subtract(v2, v0);

        Vector2 deltaUV1 = Vector2Subtract(uv1, uv0);
        Vector2 deltaUV2 = Vector2Subtract(uv2, uv0);

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        Vector3 tangent = Vector3Scale(
            Vector3Subtract(
                Vector3Scale(deltaPos1, deltaUV2.y), Vector3Scale(deltaPos2, deltaUV1.y)
            ),
            r
        );
        Vector3 bitangent = Vector3Scale(
            Vector3Subtract(
                Vector3Scale(deltaPos2, deltaUV1.x), Vector3Scale(deltaPos1, deltaUV2.x)
            ),
            r
        );

        tan1[index0] = Vector3Add(tan1[index0], tangent);
        tan1[index1] = Vector3Add(tan1[index1], tangent);
        tan1[index2] = Vector3Add(tan1[index2], tangent);

        tan2[index0] = Vector3Add(tan2[index0], bitangent);
        tan2[index1] = Vector3Add(tan2[index1], bitangent);
        tan2[index2] = Vector3Add(tan2[index2], bitangent);
    }

    // Compute tangents considering normals
    for (int i = 0; i < mesh->vertexCount; i++) {
        Vector3 n = {
            mesh->normals[i * 3 + 0], mesh->normals[i * 3 + 1], mesh->normals[i * 3 + 2]
        };
        Vector3 t = tan1[i];

        // Gram-Schmidt orthogonalize
        Vector3 tangent = Vector3Normalize(
            Vector3Subtract(t, Vector3Scale(n, Vector3DotProduct(n, t)))
        );

        // Calculate handedness (bitangent sign)
        float w = (Vector3DotProduct(Vector3CrossProduct(n, t), tan2[i]) < 0.0f) ? -1.0f
                                                                                 : 1.0f;

        mesh->tangents[i * 4 + 0] = tangent.x;
        mesh->tangents[i * 4 + 1] = tangent.y;
        mesh->tangents[i * 4 + 2] = tangent.z;
        mesh->tangents[i * 4 + 3] = w;
    }

    RL_FREE(tan1);
    RL_FREE(tan2);

    if (mesh->vboId != NULL) {
        if (mesh->vboId[SHADER_LOC_VERTEX_TANGENT] != 0) {
            // Update existing vertex buffer
            rlUpdateVertexBuffer(
                mesh->vboId[SHADER_LOC_VERTEX_TANGENT],
                mesh->tangents,
                mesh->vertexCount * 4 * sizeof(float),
                0
            );
        } else {
            // Load a new tangent attributes buffer
            mesh->vboId[SHADER_LOC_VERTEX_TANGENT] = rlLoadVertexBuffer(
                mesh->tangents, mesh->vertexCount * 4 * sizeof(float), false
            );
        }

        rlEnableVertexArray(mesh->vaoId);
        rlSetVertexAttribute(
            RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT, 4, RL_FLOAT, 0, 0, 0
        );
        rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT);
        rlDisableVertexArray();
    }

    TRACELOG(LOG_INFO, "MESH: Tangents data computed and uploaded for provided mesh");
}

Mesh gen_mesh_plane(int resolution) {
    Mesh mesh = GenMeshPlane(1.0, 1.0, resolution, resolution);
    gen_mesh_tangents(&mesh);

    return mesh;
}

Mesh gen_mesh_cube() {
    Mesh mesh = GenMeshCube(1.0, 1.0, 1.0);
    gen_mesh_tangents(&mesh);

    return mesh;
}

Mesh gen_mesh_sphere(int n_rings, int n_slices) {
    Mesh mesh = GenMeshSphere(0.5, n_rings, n_slices);

    // TODO: Fix my gen_mesh_tangents function,
    // for some reason it doesn't work for sphere now, then use it here.
    GenMeshTangents(&mesh);

    return mesh;
}

}  // namespace soft_tissues::utils
