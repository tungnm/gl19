#version 460

uniform vec4 LightPosView;

// for transform to shadow map project coord:
uniform mat4 inverseVThenLightMVP;

in vec2 texCord;

out vec4 outFragColor;

vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

// texture sampler: read from textures of G-buffer
// layout (binding=0) means set the texture unit 0 for this sampler2D gBufferPosition
layout (binding=0) uniform sampler2D gBufferPosition;
layout (binding=1) uniform sampler2D gBufferNormal;
layout (binding=2) uniform sampler2D gBufferColor;
layout (binding=3) uniform sampler2D ao;
layout (binding=4) uniform sampler2D shadowMap;


float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
    // perform perspective divide. This returns the fragment's light-space position in the range [-1,1].
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Because the depth from the depth map is in the range [0,1] and we also want to use
    // projCoords to sample from the depth map so we transform the NDC coordinates to the range [0,1]:
    projCoords = projCoords * 0.5 + 0.5; 
    
    float currentDepth = projCoords.z;
    
     
    float visibility = 0.8;
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
    // sample to get data from G-buffer: position, normal, object color
    vec3 FragPosView = texture(gBufferPosition, texCord).rgb;
    vec3 FragNormView = texture(gBufferNormal, texCord).rgb;
    vec3 FragColor = texture(gBufferColor, texCord).rgb;
    
    vec3 v = normalize(-FragPosView);
    vec3 s = normalize(LightPosView.xyz - FragPosView);

    float diffuseIntensity = max(0,dot(s, FragNormView));
        
    vec3 diffuse = vec3(0,0,0);
    vec3 specular = vec3(0,0,0);
    
    diffuse = diffuseIntensity * FragColor;
    
    if (diffuseIntensity > 0)
    {
        //blinn-phong use half way vector h
        vec3 h = normalize(v + s);
            
        //blinn-phong
        float specularIntensity = pow(max(0, dot(h, FragNormView)), 9);
            
        specular = specularIntensity * vec3(0.8);
    }
    
    //shadow map check:
    vec4 FragPosLightSpace = inverseVThenLightMVP * vec4(FragPosView, 1.0);
    float bias = 0.058 * (1.0 - diffuseIntensity) + 0.002;  
    
    float shadow = ShadowCalculation(FragPosLightSpace, bias);    
    
    float ao = texture(ao, texCord).r;
    vec3 ambient = 0.3 * ao * FragColor;
    outFragColor = vec4(ambient + shadow * (diffuse + specular), 1.0);

}