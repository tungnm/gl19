#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;

out vec3 posInView;
out vec3 normInView;

void main()
{
    
    // transform to view space and send to fragment shader to write to G-buffer
    posInView = (ModelView * vec4(VertexPosition, 1.0)).xyz;
    normInView = normalize(normalToView * VertexNormal);
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}