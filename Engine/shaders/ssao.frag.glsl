#version 460

uniform vec4 LightPosView;
in vec2 texCord;

out float outFragColor;

// texture sampler: read from textures of G-buffer
// layout (binding=0) means set the texture unit 0 for this sampler2D
layout (binding=0) uniform sampler2D gBufferPosition;

uniform mat4 projectionMatrix;
const int MAX_KERNEL_SIZE = 16;
uniform vec3 gKernel[MAX_KERNEL_SIZE];

void main()
{
    
    // grab view pos from Gbuffer
    vec3 FragPosView = texture(gBufferPosition, texCord).rgb;
    
    // SSAO calculation
    float AO = 0.0;
    
    for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {
        vec3 samplePos = FragPosView + gKernel[i]; // generate a random point
        
        vec4 offset = vec4(samplePos, 1.0); // make it a 4-vector
        offset = projectionMatrix * offset; // project on the near clipping plane
        
        vec3 projCoords = offset.xyz / offset.w;
        projCoords = projCoords * 0.5 + 0.5; 
   
        float sampleDepth = texture(gBufferPosition, projCoords.xy).z;

        AO += (sampleDepth >= samplePos.z + 0.02 ? 1.0 : 0.0);  
    }

    AO = 1.0 - AO/16;
    outFragColor = AO;
}