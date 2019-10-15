#version 460

in vec3 Color;
in vec4 FragPosLightSpace;
in float bias;
uniform sampler2D shadowMap;

out vec4 FragColor;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide. This returns the fragment's light-space position in the range [-1,1].
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Because the depth from the depth map is in the range [0,1] and we also want to use
    // projCoords to sample from the depth map so we transform the NDC coordinates to the range [0,1]:
    projCoords = projCoords * 0.5 + 0.5; 
    
    float shadowMapDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    //float bias = 0.005;
    float shadow = currentDepth - bias > shadowMapDepth  ? 0.9 : 0.0;
    
    return shadow;
}

void main()
{
    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);    

    FragColor = vec4(vec3(0.1, 0.1, 0.1) + (1.0 - shadow) * Color, 1.0);
    
}