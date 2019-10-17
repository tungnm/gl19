#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCord;
layout (location = 3) in vec4 VertexTangent;

uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;
uniform vec4 LightPosView;

out vec3 sTanSpace;
out vec3 vTanSpace;
out vec2 TexCord;

void main()
{
    // transform to eye space
    vec3 n = normalize(normalToView * VertexNormal);
    vec3 tangent = normalize(normalToView * VertexTangent.xyz);
    vec3 bitangent = normalize(cross(n, tangent)) * VertexTangent.w;
    vec3 positionEyeSpace = vec3(ModelView * vec4(VertexPosition, 1.0));
    
    //tbn matrix to transform to tangent space:
    mat3 tbn = mat3(
    tangent.x, bitangent.x, n.x,
    tangent.y, bitangent.y, n.y,
    tangent.z, bitangent.z, n.z);
    
    // transform to tangent space:
    sTanSpace = tbn * (LightPosView.xyz - positionEyeSpace); 
    vTanSpace = tbn * normalize(-positionEyeSpace);
    
    TexCord = VertexTexCord;
    
    
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
}