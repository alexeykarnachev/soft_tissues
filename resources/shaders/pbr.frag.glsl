// Constants
const int MAX_N_LIGHTS = 16;
const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;

// Structs
struct Light {
    vec3 position;
    int type;
    vec3 color;
    float intensity;
    vec3 attenuation;
    vec3 direction;
};

// Inputs
in vec3 v_world_pos;
in vec2 v_tex_coord;
in vec3 v_normal;
in mat3 v_tbn;

// Uniforms
uniform vec2 u_tiling;

uniform int u_use_normal_map;

uniform sampler2D u_albedo_map;
uniform sampler2D u_metalness_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_occlusion_map;

uniform float u_ambient_intensity;
uniform vec3 u_ambient_color;
uniform vec3 u_camera_pos;
uniform int u_n_lights;
uniform Light u_lights[MAX_N_LIGHTS];

out vec4 f_color;

float mock_usage() {
    float f = 0.0;
    vec2 uv = vec2(0.0, 0.0);

    f += v_world_pos.x;
    f += v_tex_coord.x;
    f += v_normal.x;
    f += v_tbn[0][0];

    f += u_tiling.x;
    f += float(u_use_normal_map);
    f += texture(u_albedo_map, uv).x;
    f += texture(u_metalness_map, uv).x;
    f += texture(u_normal_map, uv).x;
    f += texture(u_roughness_map, uv).x;
    f += texture(u_occlusion_map, uv).x;

    f += u_ambient_intensity;
    f += u_ambient_color.x;
    f += u_camera_pos.x;
    f += float(u_n_lights);

    if (u_n_lights > 0) {
        f += u_lights[0].intensity;
    }

    return abs(0.0000001 * f);
}

vec3 SchlickFresnel(float hDotV, vec3 refl) {
    return refl + (1.0 - refl) * pow(1.0 - hDotV, 5.0);
}

float GgxDistribution(float nDotH, float roughness) {
    float a = roughness * roughness * roughness * roughness;
    float d = nDotH * nDotH * (a - 1.0) + 1.0;
    d = PI * d * d;
    return a / max(d, 0.0000001);
}

float GeomSmith(float nDotV, float nDotL, float roughness) {
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    float ik = 1.0 - k;
    float ggx1 = nDotV / (nDotV * ik + k);
    float ggx2 = nDotL / (nDotL * ik + k);
    return ggx1 * ggx2;
}

void main() {
    vec2 uv = fract(u_tiling * v_tex_coord);

    vec3 view_dir = normalize(v_world_pos - u_camera_pos);
    vec3 albedo = texture(u_albedo_map, uv).rgb;
    float metallic = clamp(texture(u_metalness_map, uv).r, 0.04, 1.0);
    float roughness = clamp(texture(u_roughness_map, uv).r, 0.04, 1.0);
    float occlusion = texture(u_occlusion_map, uv).r;
    vec3 base_reflection = mix(vec3(0.04), albedo.rgb, metallic);

    vec3 normal = normalize(v_normal);
    if (u_use_normal_map == 1) {
        normal = texture(u_normal_map, uv).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(v_tbn * normal);
    }

    // -------------------------------------------------------------------
    // total light
    vec3 light_total = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < u_n_lights; ++i) {
        Light light = u_lights[i];

        float dist;
        vec3 light_dir;
        float attenuation;
        switch (light.type) {
            case POINT_LIGHT:
            {
                dist = length(v_world_pos - light.position);
                light_dir = normalize(v_world_pos - light.position);
                attenuation = dot(light.attenuation, vec3(1.0, dist, dist * dist));
                attenuation = 1.0 / attenuation;
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
        vec3 bisect = -normalize(view_dir + light_dir);

        // Cook-Torrance BRDF distribution function
        float nDotV = clamp(dot(v_normal, -view_dir), 0.0, 1.0);
        float nDotL = clamp(dot(v_normal, -light_dir), 0.0, 1.0);
        float hDotV = clamp(dot(bisect, -view_dir), 0.0, 1.0);
        float nDotH = clamp(dot(v_normal, bisect), 0.0, 1.0);
        float D = GgxDistribution(nDotH, roughness);
        float G = GeomSmith(nDotV, nDotL, roughness);
        vec3 F = SchlickFresnel(hDotV, base_reflection);

        vec3 spec = (D * G * F) / (4.0 * nDotV * nDotL);
        vec3 kD = (1.0 - F) * (1.0 - metallic);

        light_total += (kD * albedo / PI + spec) * radiance * nDotL;
    }

    vec3 ambient_total = (u_ambient_color + albedo) * u_ambient_intensity * 0.5;
    vec3 color = ambient_total + light_total * occlusion;

    f_color = vec4(color, 1.0 + mock_usage());
}
