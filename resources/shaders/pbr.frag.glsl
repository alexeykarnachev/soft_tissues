in vec3 v_world_pos;
in vec3 v_model_pos;
in vec2 v_tex_coord;
in vec3 v_normal;

uniform sampler2D u_albedo_map;
uniform sampler2D u_metalness_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_occlusion_map;
uniform sampler2D u_height_map;

out vec4 f_color;

void main() {
    vec3 final_color = vec3(1.0, 0.0, 1.0);

    f_color = vec4(final_color, 1.0);
}
