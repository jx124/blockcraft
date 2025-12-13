#version 460 core

in vec2 texCoord;
flat in int textureIndex;
flat in int face;

uniform sampler2DArray textureId;

out vec4 fragment;

// faces: TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK
float light_array[6] = {1.0, 0.6, 0.8, 0.8, 0.8, 0.8};

void main() {
    vec4 light = vec4(vec3(light_array[face]), 1.0);
    fragment = light * texture(textureId, vec3(texCoord, textureIndex));
};

