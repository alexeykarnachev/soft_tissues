const float PI = 3.14159265358979323846;
const float EPSILON = 0.000001;

const int MAX_N_LIGHTS = 4;
const int POINT_LIGHT = 0;
const int DIRECTIONAL_LIGHT = 1;
const int SPOT_LIGHT = 2;
const int AMBIENT_LIGHT = 3;

struct Light {
    int type;
    int casts_shadows;

    float intensity;
    float inner_cutoff;
    float outer_cutoff;

    vec3 position;
    vec3 attenuation;
    vec3 direction;
    vec3 color;

    mat4 vp_mat;
};
