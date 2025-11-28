#version 330
in vec2 texCoord;

uniform sampler2D textureId;

out vec4 fragment;

void main() {
    fragment = texture(textureId, texCoord);
};

