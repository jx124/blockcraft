#version 330
layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTexCoord;

uniform mat4 MVP;

out vec2 texCoord;

void main() {
    gl_Position = MVP * vec4(vPos, 0.0, 1.0);
    texCoord = vTexCoord;
};

