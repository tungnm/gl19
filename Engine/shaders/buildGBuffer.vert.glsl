#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 MVP;

out vec3 RawPosition;
out vec3 RawNormal;

void main()
{

    // forward raw data, unchanged(no transformation) from VBO
    RawPosition = VertexPosition;
    RawNormal = VertexNormal;
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}