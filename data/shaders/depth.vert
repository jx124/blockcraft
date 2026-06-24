#version 460 core
layout (location = 0) in uint vData;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // unpack data
    int x = int((vData >> 27) & 0x1F);
    int y = int((vData >> 22) & 0x1F);
    int z = int((vData >> 13) & 0x1FF);
    int u = int((vData >> 12) & 0x1);
    int v = int((vData >> 11) & 0x1);
    vec4 vPos = vec4(x, y, z, 1.0);

    gl_Position = projection * view * model * vPos;
}
