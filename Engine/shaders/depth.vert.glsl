#version 460

layout (location = 0) in vec3 VertexPosition;

uniform vec4 LightPosView;
uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;

void main()
{    
    gl_Position = MVP * vec4(VertexPosition,1.0);   
}