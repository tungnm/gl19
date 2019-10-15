#version 460

// need special sampler type to sample the depth buffer
uniform sampler2DShadow shadowMap;

in vec3 Color;
in vec4 VertexPosLightSpace;
out vec4 FragColor;
void main()
{
    float shadow = textureProj(ShadowMap, ShadowCoord);
    FragColor = vec4(Color * shadow, 1.0);
    
}