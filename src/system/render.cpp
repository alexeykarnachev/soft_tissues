#include "render.hpp"

#include "../globals.hpp"
#include "../pbr.hpp"
#include "lighting.hpp"
#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"

namespace soft_tissues::system::render {

void begin_frame(pbr::PBRShader &pbr_shader) {
    pbr_shader.set_shadow_map_pass(globals::RENDER_STATE.is_shadow_map_pass);

    Matrix mat = MatrixInvert(rlGetMatrixModelview());
    pbr_shader.set_camera_pos({mat.m12, mat.m13, mat.m14});

    if (!globals::RENDER_STATE.is_shadow_map_pass) {
        pbr_shader.set_light_enabled(globals::RENDER_STATE.is_light_enabled);
        pbr_shader.set_shadow_map_bias(globals::RENDER_STATE.shadow_map_bias);
        pbr_shader.set_shadow_map_max_dist(globals::RENDER_STATE.shadow_map_max_dist);

        if (globals::RENDER_STATE.is_light_enabled) {
            lighting::set_light_uniforms(pbr_shader);
        }
    }
}

void draw_mesh(Mesh mesh, pbr::MaterialPBR material_pbr, Color constant_color, Matrix matrix) {
    Material material = material_pbr.get_material();
    pbr::PBRShader &pbr_shader = material_pbr.get_pbr_shader();

    pbr_shader.set_tiling(material_pbr.get_tiling());
    pbr_shader.set_displacement_scale(material_pbr.get_displacement_scale());

    if (!globals::RENDER_STATE.is_shadow_map_pass) {
        pbr_shader.set_constant_color(constant_color);
    }

    DrawMesh(mesh, material, matrix);
}

}  // namespace soft_tissues::system::render
