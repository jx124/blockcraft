#version 460
layout (location = 0) in vec2 vPos;

uniform float aspectRatio;

void main() {
    gl_Position = vec4(vPos.x / aspectRatio, vPos.y, 0.0, 1.0);
}
