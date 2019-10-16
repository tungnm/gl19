#version 460

in vec3 Color;
in vec4 FragPosLightSpace;
in float bias;
uniform sampler2D shadowMap;

out vec4 FragColor;

//test
vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide. This returns the fragment's light-space position in the range [-1,1].
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Because the depth from the depth map is in the range [0,1] and we also want to use
    // projCoords to sample from the depth map so we transform the NDC coordinates to the range [0,1]:
    projCoords = projCoords * 0.5 + 0.5; 
    
    float currentDepth = projCoords.z;
    
     
    float visibility = 1.0;
    //test
    for (int i=0;i<4;i++){
  if ( texture( shadowMap, projCoords.xy + poissonDisk[i]/700.0 ).z  < currentDepth-bias ){
    visibility-=0.2;
  }
  
  
}
return visibility;
}
    
    
    
 
void main()
{
    // calculate shadow
    float shadow = ShadowCalculation(FragPosLightSpace);    

    FragColor = vec4(vec3(0.1, 0.1, 0.1) + shadow * Color, 1.0);
    
}