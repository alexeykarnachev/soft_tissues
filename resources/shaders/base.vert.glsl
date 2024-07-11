// Inputs
in vec3 a_position;
in vec2 a_tex_coord;
in vec3 a_normal;

// Uniforms
uniform mat4 u_mvp_mat;
uniform mat4 u_model_mat;
uniform mat4 u_normal_mat;

// Outputs to fragment shader
out vec3 v_world_pos;
out vec2 v_tex_coord;
out vec3 v_normal;
out mat3 v_tbn;

void get_basis(out vec3 tangent, out vec3 bitangent, in vec3 normal) {
    bitangent = vec3(0.0, 1.0, 0.0);

    float normalDotUp = dot(normal, bitangent);

    if (normalDotUp == 1.0) {
        bitangent = vec3(0.0, 0.0, -1.0);
    } else if (normalDotUp == -1.0) {
        bitangent = vec3(0.0, 0.0, 1.0);
    }

    tangent = normalize(cross(bitangent, normal));
    bitangent = normalize(cross(normal, tangent));
}

vec3 mat4_by_vec3(mat4 mat, vec3 vec) {
    return vec3(mat * vec4(vec, 1.0));
}

void main() {
    v_world_pos = mat4_by_vec3(u_model_mat, a_position);
    v_tex_coord = a_tex_coord;
    v_normal = normalize(mat4_by_vec3(u_normal_mat, a_normal));

    vec3 tangent;
    vec3 bitangent;
    get_basis(tangent, bitangent, v_normal);
    v_tbn = mat3(tangent, bitangent, v_normal);

    gl_Position = u_mvp_mat * vec4(a_position, 1.0);
}
