#version 460 core
layout (location = 0) in uint vData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform vec3 cameraPos;

out vec2 texCoord;
flat out int textureIndex;
flat out int face;
out vec4 worldDistance;
out vec4 fragPosLightSpace;

void main() {
    // unpack data
    int x = int((vData >> 27) & 0x1F);
    int y = int((vData >> 22) & 0x1F);
    int z = int((vData >> 13) & 0x1FF);
    int u = int((vData >> 12) & 0x1);
    int v = int((vData >> 11) & 0x1);
    int texture_index = int((vData >> 3) & 0xFF);
    int vFace = int((vData >> 0) & 0x7);

    vec4 modelPos = model * vec4(x, y, z, 1.0);

    gl_Position = projection * view * modelPos;
    texCoord = vec2(u, v);
    textureIndex = texture_index;
    face = vFace;
    worldDistance = vec4(cameraPos, 1.0) - modelPos;
    fragPosLightSpace = lightSpaceMatrix * modelPos;
}

