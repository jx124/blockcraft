#version 460 core

in vec2 texCoord;
flat in int textureIndex;
flat in int face;
in vec4 worldDistance;
in vec4 fragPosWorldSpace;

uniform sampler2DArray textureId;
uniform sampler2DArray depthMap;
uniform float renderRadius;
uniform vec3 lightPos;
uniform mat4 view;

layout (std140, binding = 0) uniform CascadedShadowMap {
    // TODO: set to final constant
    mat4 lightSpaceMatrices[16];
    float cascadeSplitPlanes[16];
    int numCascades;
};

out vec4 fragment;

// faces: TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK
float light_array[6] = {1.0, 0.6, 0.8, 0.8, 0.8, 0.8};
vec3 normals[6] = {
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 0.0, -1.0),
    vec3(-1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, -1.0, 0.0),
    vec3(0.0, 1.0, 0.0),
};

float fog_factor(vec4 worldDistance) {
    float dist = length(worldDistance.xyz);

    float start = 0.8 * renderRadius;
    float end = 0.9 * renderRadius;

    if (dist > end) {
        return 1.0;
    } else if (dist < start) {
        return 0.0;
    } else {
        return (dist - start) / (end - start);
    }
}

float shadow_calculation(vec4 fragPosLightSpace, int face, int layer) {
    // perspective divide then rescale to [0, 1]
    vec3 proj_coords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    proj_coords = proj_coords * 0.5f + 0.5f;

    float current_depth = proj_coords.z;
    // no shadows outside of depth map range
    if (current_depth < 0.0f || current_depth > 1.0f) {
        return 0.0f;
    }

    float diffuse_factor = dot(normals[face], normalize(lightPos));
    // occluded face should be in shadow
    if (diffuse_factor < 0.0f) {
        return 1.0f;
    }
    float bias = mix(0.01f, 0.0f, diffuse_factor);

    // percentage-closer filtering in 3x3 region to smooth out shadows
    float shadow = 0.0f;
    vec2 texel_size = 1.0f / vec2(textureSize(depthMap, 0));
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcf_depth = texture(depthMap, vec3(proj_coords.xy + vec2(x, y) * texel_size, layer)).r; 
            shadow += current_depth - bias > pcf_depth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;
    return shadow;
}

void main() {
    vec4 frag_pos_view_space = view * fragPosWorldSpace;
    float depth = -frag_pos_view_space.z;

    int layer = -1;
    for (int i = 0; i < numCascades; i++) {
        if (depth < cascadeSplitPlanes[i]) {
            layer = i;
            break;
        }
    }
    if (layer == -1) {
        layer = numCascades - 1;
    }

    vec4 frag_pos_light_space = lightSpaceMatrices[layer] * fragPosWorldSpace;
    float shadow_factor = shadow_calculation(frag_pos_light_space, face, layer);

    float ambient = 0.8f * light_array[face];
    float diffuse = dot(normals[face], normalize(lightPos));
    vec4 light = vec4(vec3(ambient + diffuse * mix(1.0f, 0.1f, shadow_factor)), 1.0f);
    vec4 fog_color = vec4(0.4f, 0.75f, 0.9f, 1.0f);

    fragment = mix(light * texture(textureId, vec3(texCoord, textureIndex)), fog_color, fog_factor(worldDistance));
};

