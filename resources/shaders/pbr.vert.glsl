// Inputs
in vec3 a_position;
in vec2 a_tex_coord;
in vec3 a_normal;

// Uniforms
uniform mat4 u_mvp_mat;
uniform mat4 u_model_mat;
uniform mat4 u_normal_mat;

uniform vec2 u_tiling;

uniform sampler2D u_height_map;
uniform float u_displacement_scale;

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
    v_normal = normalize(mat4_by_vec3(u_normal_mat, a_normal));

    v_tex_coord = u_tiling * a_tex_coord;
    float height = texture(u_height_map, v_tex_coord).r;
    vec3 position = a_position + a_normal * height * u_displacement_scale;

    v_world_pos = mat4_by_vec3(u_model_mat, position);

    vec3 tangent;
    vec3 bitangent;
    get_basis(tangent, bitangent, v_normal);
    v_tbn = mat3(tangent, bitangent, v_normal);

    gl_Position = u_mvp_mat * vec4(position, 1.0);
}
