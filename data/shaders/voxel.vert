#version 460 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexCoord;
layout (location = 2) in int vTextureIndex;
layout (location = 3) in int vFace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

out vec2 texCoord;
flat out int textureIndex;
flat out int face;
out vec4 worldDistance;

void main() {
    gl_Position = projection * view * model * vec4(vPos, 1.0);
    texCoord = vTexCoord;
    textureIndex = vTextureIndex;
    face = vFace;
    worldDistance = vec4(cameraPos, 1.0) - model * vec4(vPos, 1.0);
}

