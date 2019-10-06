#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;

out vec3 Position;
out vec3 Normal;

void main()
{
    Normal = normalize(normalToView * VertexNormal);
    Position = (ModelView * vec4(VertexPosition, 1.0)).xyz;
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}