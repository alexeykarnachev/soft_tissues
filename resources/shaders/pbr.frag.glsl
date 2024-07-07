// -----------------------------------------------------------------------
// constants
const int MAX_N_LIGHTS = 16;
const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;

// -----------------------------------------------------------------------
// light structure
struct Light {
    vec3 position;

    int type;
    vec3 color;
    float intensity;

    // point
    vec3 attenuation;

    // directional
    vec3 direction;
};

vec3 SchlickFresnel(float hDotV, vec3 reflectance) {
    return reflectance + (1.0 - reflectance) * pow(1.0 - hDotV, 5.0);
}

float GgxDistribution(float nDotH, float roughness) {
    float a = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (a - 1.0) + 1.0;
    d = PI * d * d;
    return a / max(d, EPSILON);
}

float GeomSmith(float nDotV, float nDotL, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    float ik = 1.0 - k;
    float ggx1 = nDotV / (nDotV * ik + k);
    float ggx2 = nDotL / (nDotL * ik + k);
    return ggx1 * ggx2;
}

// -----------------------------------------------------------------------
// inputs
in vec3 v_world_pos;
in vec2 v_tex_coord;
in vec3 v_normal;
in mat3 v_tbn;

// -----------------------------------------------------------------------
// textures
uniform vec2 u_tiling;
uniform int u_use_normal_map;

uniform sampler2D u_albedo_map;
uniform sampler2D u_metalness_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_occlusion_map;

// -----------------------------------------------------------------------
// lighting
uniform float u_ambient_intensity;
uniform vec3 u_ambient_color;

uniform vec3 u_camera_pos;
uniform int u_n_lights;
uniform Light u_lights[MAX_N_LIGHTS];

out vec4 f_color;

void main() {
    vec2 uv = v_tex_coord * u_tiling;

    vec3 albedo = texture(u_albedo_map, uv).rgb;
    float metalness = texture(u_metalness_map, uv).r;
    float roughness = texture(u_roughness_map, uv).r;
    float occlusion = texture(u_occlusion_map, uv).r;

    vec3 normal = normalize(v_normal);
    if (u_use_normal_map == 1) {
        normal = texture(u_normal_map, uv).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        normal = normalize(normal * v_tbn);
    }

    vec3 view_dir = normalize(v_world_pos - u_camera_pos);
    vec3 base_reflectance = mix(vec3(0.04), albedo.rgb, metalness);
    vec3 total_light = vec3(0.0);

    for (int i = 0; i < u_n_lights; i++) {
        Light light = u_lights[i];

        vec3 light_dir;
        float attenuation;
        switch (light.type) {
            case POINT_LIGHT:
            {
                light_dir = normalize(v_world_pos - light.position);

                float dist = length(light.position - v_world_pos);
                vec3 factor = vec3(1.0, dist, dist * dist);
                attenuation = 1.0 / dot(light.attenuation, factor);
                break;
            }
            case DIRECTIONAL_LIGHT:
            {
                light_dir = light.direction;
                attenuation = 1.0;
                break;
            }
        }

        vec3 radiance = light.color * light.intensity * attenuation;
        vec3 bisect_dir = -normalize(view_dir + light_dir);

        float nDotV = max(dot(normal, -view_dir), EPSILON);
        float nDotL = max(dot(normal, -light_dir), EPSILON);
        float hDotV = max(dot(bisect_dir, -view_dir), 0.0);
        float nDotH = max(dot(normal, -bisect_dir), 0.0);
        float D = GgxDistribution(nDotH, roughness);
        float G = GeomSmith(nDotV, nDotL, roughness);
        vec3 F = SchlickFresnel(hDotV, base_reflectance);

        vec3 specular = (D * G * F) / (4.0 * nDotV * nDotL);
        vec3 kD = (vec3(1.0) - F) * (1.0 - metalness);

        total_light += ((kD * albedo / PI + specular) * radiance * nDotL);
    }

    vec3 total_ambient = (u_ambient_color + albedo) * u_ambient_intensity * 0.5;
    vec3 color = total_ambient + total_light * occlusion;

    // hdr & gamma
    color = pow(color, color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    f_color = vec4(color, 1.0);
}
