#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 2) in vec2 VertexTexCord;

out vec2 texCord;

void main()
{
    texCord = vec2(VertexTexCord.x, -VertexTexCord.y);
    // quad's vertexs are in clip space. No transformation needed.
    gl_Position = vec4(VertexPosition,1.0);
}