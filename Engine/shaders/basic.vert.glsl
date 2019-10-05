#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform vec4 LightPosView;
uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;

out vec3 Color;
//hello
void main()
{
vec3 Kd = vec3(0.9f, 0.5f, 0.3f);
    vec3 Ld = vec3(1.0f, 1.0f, 1.0f);

    vec3 tnorm = normalize(vec3( normalToView * VertexNormal));
vec4 camCoords = ModelView * vec4(VertexPosition,1.0);
vec3 s = normalize(vec3(LightPosView - camCoords));
// The diffuse shading equation
Color = Ld * Kd * max( dot( s, tnorm ), 0.0 );

    gl_Position = MVP * vec4(VertexPosition, 1.0);
    //gl_Position = vec4(VertexPosition, 1.0);
}