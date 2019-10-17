#version 460

// send object color directly to fragment shader, no need to interpolate from vertex shader
uniform vec3 ObjectColor;

in vec3 RawPosition;
in vec3 RawNormal;

layout (location = 0) out vec3 GBufferPos;
layout (location = 1) out vec3 GBufferNorm;
layout (location = 2) out vec3 GBufferColor;

void main()
{
    // output to G-Buffer with multiple rendering targets(writing to 3 textures at once)
    GBufferPos = RawPosition;
    GBufferNorm = RawNormal;
    GBufferColor = ObjectColor;
}