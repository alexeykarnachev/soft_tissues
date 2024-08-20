// -----------------------------------------------------------------------
// Inputs
in vec3 v_world_pos;
in vec2 v_tex_coord;
in vec3 v_normal;
in mat3 v_tbn;

// NOTE: This represents the rasterized vertex position in a ndc light space.
// Since there are more than 1 light, these positions are stored in the array.
in vec4 v_light_positions[MAX_N_LIGHTS];

// -----------------------------------------------------------------------
// Uniforms
uniform sampler2D u_shadow_maps[MAX_N_SHADOW_MAPS];

uniform sampler2D u_albedo_map;
uniform sampler2D u_metalness_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_roughness_map;
uniform sampler2D u_occlusion_map;

uniform vec4 u_constant_color;

uniform int u_is_shadow_map_pass;
uniform int u_is_light_enabled;
uniform float u_shadow_map_bias;
uniform float u_shadow_map_max_dist;
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

    f += texture(u_albedo_map, uv).x;
    f += texture(u_metalness_map, uv).x;
    f += texture(u_normal_map, uv).x;
    f += texture(u_roughness_map, uv).x;
    f += texture(u_occlusion_map, uv).x;

    f += u_camera_pos.x;
    f += float(u_n_lights);

    f += u_shadow_map_bias;

    if (u_n_lights > 0) {
        f += u_lights[0].intensity;
    }

    return abs(EPSILON * f);
}

vec3 SchlickFresnel(float hDotV, vec3 refl) {
    return refl + (1.0 - refl) * pow(1.0 - hDotV, 5.0);
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

vec3 get_shadow_map_color() {
    float dist_to_camera = distance(u_camera_pos, v_world_pos);
    float depth = dist_to_camera / u_shadow_map_max_dist;
    return vec3(depth);
}

vec3 get_albedo_color() {
    vec2 uv = v_tex_coord;
    vec3 color = texture(u_albedo_map, uv).rgb;
    color = mix(color, u_constant_color.rgb, u_constant_color.a);

    return color;
}

vec3 get_pbr_color() {
    vec2 uv = v_tex_coord;
    vec3 albedo = texture(u_albedo_map, uv).rgb;

    vec3 view_dir = normalize(v_world_pos - u_camera_pos);
    float metallic = clamp(texture(u_metalness_map, uv).r, 0.04, 1.0);
    float roughness = clamp(texture(u_roughness_map, uv).r, 0.04, 1.0);
    float occlusion = texture(u_occlusion_map, uv).r;
    vec3 base_reflection = mix(vec3(0.04), albedo.rgb, metallic);

    vec3 normal = texture(u_normal_map, uv).rgb;
    if (length(normal) > EPSILON) {
        normal = v_tbn * normalize(normal * 2.0 - 1.0);
    } else {
        normal = v_normal;
    }
    normal = normalize(normal);

    // -------------------------------------------------------------------
    // total light
    vec3 light_total = vec3(0.0, 0.0, 0.0);
    vec3 ambient_total = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < u_n_lights; ++i) {
        Light light = u_lights[i];
        float dist = length(v_world_pos - light.position);

        bool is_in_shadow = false;
        if (light.casts_shadows == 1) {
            vec4 ndc = v_light_positions[i];
            vec2 uv = 0.5 * ((ndc.xy / ndc.w) + 1.0);
            if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0) {
                is_in_shadow = true;
            } else {
                float depth = u_shadow_map_max_dist * texture(u_shadow_maps[i], uv).r;
                is_in_shadow = depth < (dist + u_shadow_map_bias);
            }
        }

        if (is_in_shadow) continue;

        vec3 light_dir;
        float attenuation;
        switch (light.type) {
            case POINT_LIGHT:
            {
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
            case SPOT_LIGHT:
            {
                light_dir = normalize(v_world_pos - light.position);
                float theta = dot(light_dir, normalize(light.direction));
                float epsilon = light.inner_cutoff - light.outer_cutoff;
                float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);
                attenuation = dot(light.attenuation, vec3(1.0, dist, dist * dist));
                attenuation = 1.0 / attenuation;
                attenuation *= intensity;
                break;
            }
            case AMBIENT_LIGHT:
            {
                ambient_total += (light.color + albedo) * light.intensity * 0.5;
                // NOTE: Ambient light doesn't compute BRDF,
                // so I just continue the iteration over lights
                continue;
            }
        }

        vec3 radiance = light.color * light.intensity * attenuation;
        vec3 bisect = -normalize(view_dir + light_dir);

        // Cook-Torrance BRDF distribution function
        float nDotV = clamp(dot(normal, -view_dir), 0.0, 1.0);
        float nDotL = clamp(dot(normal, -light_dir), 0.0, 1.0);
        float hDotV = clamp(dot(bisect, -view_dir), 0.0, 1.0);
        float nDotH = clamp(dot(normal, bisect), 0.0, 1.0);
        float D = GgxDistribution(nDotH, roughness);
        float G = GeomSmith(nDotV, nDotL, roughness);
        vec3 F = SchlickFresnel(hDotV, base_reflection);

        vec3 spec = (D * G * F) / max(4.0 * nDotV * nDotL, EPSILON);
        vec3 kD = (1.0 - F) * (1.0 - metallic);

        light_total += (kD * albedo / PI + spec) * radiance * nDotL;
    }

    vec3 color = ambient_total + light_total * occlusion;
    color = mix(color, u_constant_color.rgb, u_constant_color.a);
    // color = color + mock_usage();

    return color;
}

void main() {
    vec3 color;

    // TODO: Introduce render type enum: SHADOW_MAP, ALBEDO, PBR
    if (u_is_shadow_map_pass == 1) {
        color = get_shadow_map_color();
    } else if (u_is_light_enabled == 1) {
        color = get_pbr_color();
    } else {
        color = get_albedo_color();
    }

    f_color = vec4(color, 1.0);
}
