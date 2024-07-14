#include "resources.hpp"

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include "utils.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>

namespace soft_tissues::resources {

using namespace utils;

Material DEFAULT_MATERIAL;
Material BRICK_WALL_MATERIAL;
Material TILED_STONE_MATERIAL;

Mesh PLANE_MESH;

static std::string get_shader_file_path(const std::string &file_name) {
    auto file_path = "resources/shaders/" + file_name;
    return file_path;
}

static std::string load_shader_src(const std::string &file_name) {
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

static Shader load_shader(
    const std::string &vs_file_name, const std::string &fs_file_name
) {
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

static void load_texture(std::string dir_path, std::string file_name, Texture *texture) {
    auto file_path = dir_path + "/" + file_name;

    if (!std::filesystem::exists(file_path)) {
        unsigned char pixels[4] = {0, 0, 0, 0};
        texture->id = rlLoadTexture(
            pixels, 1, 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1
        );
        TraceLog(LOG_INFO, "Texture is missing: %s", file_path.c_str());
        return;
    }

    *texture = LoadTexture(file_path.c_str());

    GenTextureMipmaps(texture);
    SetTextureFilter(*texture, TEXTURE_FILTER_TRILINEAR);
    SetTextureWrap(*texture, TEXTURE_WRAP_REPEAT);
}

static Material load_pbr_material(std::string textures_dir_path) {
    Shader shader = load_shader("pbr.vert.glsl", "pbr.frag.glsl");
    Material material = LoadMaterialDefault();

    // -------------------------------------------------------------------
    // shader locations

    // vertex attributes
    shader.locs[SHADER_LOC_VERTEX_POSITION] = get_attribute_loc(shader, "a_position");
    shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = get_attribute_loc(shader, "a_tex_coord");
    shader.locs[SHADER_LOC_VERTEX_NORMAL] = get_attribute_loc(shader, "a_normal");
    shader.locs[SHADER_LOC_VERTEX_TANGENT] = get_attribute_loc(shader, "a_tangent");

    // uniforms
    shader.locs[SHADER_LOC_MATRIX_MVP] = get_uniform_loc(shader, "u_mvp_mat");
    shader.locs[SHADER_LOC_MATRIX_MODEL] = get_uniform_loc(shader, "u_model_mat");
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = get_uniform_loc(shader, "u_normal_mat");

    shader.locs[SHADER_LOC_MAP_ALBEDO] = get_uniform_loc(shader, "u_albedo_map");
    shader.locs[SHADER_LOC_MAP_METALNESS] = get_uniform_loc(shader, "u_metalness_map");
    shader.locs[SHADER_LOC_MAP_NORMAL] = get_uniform_loc(shader, "u_normal_map");
    shader.locs[SHADER_LOC_MAP_ROUGHNESS] = get_uniform_loc(shader, "u_roughness_map");
    shader.locs[SHADER_LOC_MAP_OCCLUSION] = get_uniform_loc(shader, "u_occlusion_map");
    shader.locs[SHADER_LOC_MAP_HEIGHT] = get_uniform_loc(shader, "u_height_map");

    material.shader = shader;

    // -------------------------------------------------------------------
    // textures
    load_texture(
        textures_dir_path, "albedo.png", &material.maps[MATERIAL_MAP_ALBEDO].texture
    );
    load_texture(
        textures_dir_path, "metalness.png", &material.maps[MATERIAL_MAP_METALNESS].texture
    );
    load_texture(
        textures_dir_path, "normal.png", &material.maps[MATERIAL_MAP_NORMAL].texture
    );
    load_texture(
        textures_dir_path, "roughness.png", &material.maps[MATERIAL_MAP_ROUGHNESS].texture
    );
    load_texture(
        textures_dir_path, "occlusion.png", &material.maps[MATERIAL_MAP_OCCLUSION].texture
    );
    load_texture(
        textures_dir_path, "height.png", &material.maps[MATERIAL_MAP_HEIGHT].texture
    );

    return material;
}

static void gen_mesh_tangents(Mesh *mesh) {
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

static Mesh gen_mesh_plane(int resolution) {
    Mesh mesh = GenMeshPlane(1.0, 1.0, resolution, resolution);
    gen_mesh_tangents(&mesh);

    return mesh;
}

void load() {
    DEFAULT_MATERIAL = LoadMaterialDefault();
    BRICK_WALL_MATERIAL = load_pbr_material("resources/textures/brick_wall/");
    TILED_STONE_MATERIAL = load_pbr_material("resources/textures/tiled_stone/");

    PLANE_MESH = gen_mesh_plane(16);
}

void unload() {
    UnloadMaterial(BRICK_WALL_MATERIAL);

    UnloadMesh(PLANE_MESH);
}

}  // namespace soft_tissues::resources
