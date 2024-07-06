in vec3 a_position;
in vec2 a_tex_coord;
in vec3 a_normal;

uniform mat4 u_mvp_mat;
uniform mat4 u_model_mat;
uniform mat4 u_normal_mat;

out vec3 v_world_pos;
out vec3 v_model_pos;
out vec2 v_tex_coord;
out vec3 v_normal;

void main() {
    v_model_pos = a_position;
    v_world_pos = (u_model_mat * vec4(a_position, 1.0)).xyz;
    v_tex_coord = a_tex_coord;
    v_normal = normalize(vec3(u_normal_mat * vec4(a_normal, 1.0)));

    gl_Position = u_mvp_mat * vec4(a_position, 1.0);
}
