#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCord;

uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;

out vec3 Position;
out vec3 Normal;
out vec2 TexCord;

void main()
{
    Normal = normalize(normalToView * VertexNormal);
    Position = (ModelView * vec4(VertexPosition, 1.0)).xyz;
    TexCord = VertexTexCord;
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}