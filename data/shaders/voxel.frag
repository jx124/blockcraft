#version 460 core

in vec2 texCoord;
flat in int textureIndex;

uniform sampler2DArray textureId;

out vec4 fragment;

void main() {
    fragment = texture(textureId, vec3(texCoord, textureIndex));
};

