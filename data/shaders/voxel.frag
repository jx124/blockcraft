#version 460 core

in vec2 texCoord;
flat in int textureIndex;
flat in int face;
in vec4 worldDistance;

uniform sampler2DArray textureId;
uniform float renderRadius;

out vec4 fragment;

// faces: TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK
float light_array[6] = {1.0, 0.6, 0.8, 0.8, 0.8, 0.8};

float fog_factor(vec4 worldDistance) {
    // get cylindrical distance
    float dist = length(worldDistance.xy);

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

void main() {
    vec4 light = vec4(vec3(light_array[face]), 1.0);
    vec4 fog_color = vec4(0.4f, 0.75f, 0.9f, 1.0f);
    fragment = mix(light * texture(textureId, vec3(texCoord, textureIndex)), fog_color, fog_factor(worldDistance));
};

