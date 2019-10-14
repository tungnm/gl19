#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform vec4 LightPosView;
uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;
uniform vec3 ObjectColor;

out vec3 Color;

void main()
{
    // transform normal to view space
    vec3 n = normalize(normalToView * VertexNormal);
    vec4 vertexPosInView = ModelView * vec4(VertexPosition, 1.0);
    vec3 s = normalize(vec3(LightPosView - vertexPosInView));
    
    float diffuseIntensity = max(0, dot(s,n));
    
    vec3 diffuse = diffuseIntensity * ObjectColor;
    
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(-vertexPosInView.xyz);
    
    vec3 r = 2 * diffuseIntensity * n - s;
      
    float specularIntensity = pow(max(0,dot(r, v)),2);
    
    vec3 specular = specularIntensity * vec3(0.8);
    
    Color = diffuse + specular;
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}