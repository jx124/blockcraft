#version 460 core
layout (location = 0) in uint vData;

uniform mat4 model;

void main()
{
    // unpack data
    int x = int((vData >> 27) & 0x1F);
    int y = int((vData >> 22) & 0x1F);
    int z = int((vData >> 14) & 0xFF);
    vec4 vPos = vec4(x, y, z, 1.0);

    gl_Position = model * vPos;
}
