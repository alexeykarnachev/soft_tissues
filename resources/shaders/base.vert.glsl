in vec3 a_position;
in vec2 a_tex_coord;
in vec3 a_normal;
in vec3 a_tangent;

uniform mat4 u_mvp_mat;
uniform mat4 u_model_mat;
uniform mat4 u_normal_mat;

out vec3 v_world_pos;
out vec2 v_tex_coord;
out vec3 v_normal;
out mat3 v_tbn;

void main() {
    v_world_pos = (u_model_mat * vec4(a_position, 1.0)).xyz;
    v_tex_coord = a_tex_coord;
    v_normal = normalize(u_normal_mat * vec4(a_normal, 1.0)).xyz;

    vec3 tangent = normalize(u_normal_mat * vec4(a_tangent, 1.0)).xyz;
    tangent = normalize(tangent - v_normal * dot(tangent, v_normal));
    vec3 binormal = cross(v_normal, tangent);
    v_tbn = transpose(mat3(tangent, binormal, v_normal));

    gl_Position = u_mvp_mat * vec4(a_position, 1.0);
}
