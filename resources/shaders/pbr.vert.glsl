// Inputs

// NOTE: input locations correspond to the raylib's default shader attrib locations
// https://github.com/raysan5/raylib/blob/5ede47618bd9f9a440af648da1b4817e51644994/src/rlgl.h#L325
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_tex_coord;
layout(location = 2) in vec3 a_normal;
// layout(location = 3) in vec3 a_color;
layout(location = 4) in vec4 a_tangent;
// layout(location = 5) in vec2 a_tex_coord;

// Uniforms
uniform mat4 u_mvp_mat;
uniform mat4 u_model_mat;
uniform mat4 u_normal_mat;

uniform vec2 u_tiling;

uniform sampler2D u_height_map;
uniform float u_displacement_scale;

uniform int u_n_lights;
uniform Light u_lights[MAX_N_LIGHTS];

// Outputs to fragment shader
out vec3 v_world_pos;
out vec2 v_tex_coord;
out vec3 v_normal;
out mat3 v_tbn;

// NOTE: This represents vertex position in a ndc light space.
// Since there are more than 1 light, these positions are stored in the array.
out vec4 v_light_positions[MAX_N_LIGHTS];

vec3 mat4_by_vec3(mat4 mat, vec3 vec) {
    return vec3(mat * vec4(vec, 1.0));
}

void main() {
    v_normal = normalize(mat4_by_vec3(u_normal_mat, a_normal));
    v_tex_coord = u_tiling * a_tex_coord;
    float height = texture(u_height_map, v_tex_coord).r;
    vec3 position = a_position + a_normal * height * u_displacement_scale;
    v_world_pos = mat4_by_vec3(u_model_mat, position);

    gl_Position = u_mvp_mat * vec4(position, 1.0);

    vec3 tangent = normalize(mat3(u_model_mat) * a_tangent.xyz);
    vec3 bitangent = normalize(cross(tangent, v_normal) * a_tangent.w);
    v_tbn = mat3(tangent, bitangent, v_normal);

    for (int i = 0; i < u_n_lights; ++i) {
        mat4 vp_mat = u_lights[i].vp_mat;
        vec4 ndc = vp_mat * u_model_mat * vec4(a_position, 1.0);
        v_light_positions[i] = ndc;
    }
}
