#version 460 core
layout (location = 0) in uint vData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

out vec2 texCoord;
flat out int textureIndex;
flat out int face;
out vec4 worldDistance;
out vec4 fragPosWorldSpace;

void main() {
    // unpack data
    int x = int((vData >> 27) & 0x1F);
    int y = int((vData >> 22) & 0x1F);
    int z = int((vData >> 14) & 0xFF);
    int u = int((vData >> 13) & 0x1);
    int v = int((vData >> 12) & 0x1);
    int texture_index = int((vData >> 5) & 0x7F);
    int vFace = int((vData >> 2) & 0x7);
    int ambient_occlusion = int(vData & 0x3);

    vec4 modelPos = model * vec4(x, y, z, 1.0);

    gl_Position = projection * view * modelPos;
    texCoord = vec2(u, v);
    textureIndex = texture_index;
    face = vFace;
    worldDistance = vec4(cameraPos, 1.0) - modelPos;
    fragPosWorldSpace = modelPos;
}

