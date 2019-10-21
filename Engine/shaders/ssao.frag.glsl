#version 460

uniform vec4 LightPosView;
in vec2 texCord;

out float outFragColor;

// texture sampler: read from textures of G-buffer
// layout (binding=0) means set the texture unit 0 for this sampler2D
layout (binding=0) uniform sampler2D gBufferPosition;
layout (binding=1) uniform sampler2D gBufferNormal;
layout (binding=2) uniform sampler2D noise;

uniform mat4 projectionMatrix;
const int MAX_KERNEL_SIZE = 32;
// array of random vec3 inside a sphere.
uniform vec3 gKernel[MAX_KERNEL_SIZE];

const vec2 noiseScale = vec2(1024.0 / 4.0, 768.0 / 4.0); // screen = 1280x720

void main()
{
    // even the vectors of gKernel is randomized, they are still the same for all
    // pixel calculation. We use another random vec3 called randomAxis, which is
    // sampled/ tiled from a small random texture. Then reflect all vectors of gKernel
    // around this randomAxis. This keep the vec3 of gKernel random for all pixels.
    vec3 randomAxis = texture(noise, texCord * noiseScale).xyz;  
    // grab view pos from Gbuffer
    outFragColor = 1.0;
    vec3 FragPosView = texture(gBufferPosition, texCord).rgb;
    
    // skip the pixels that are the background, so position is (0,0,0)
    if (length(FragPosView) != 0)
    {
        vec3 FragNormView = texture(gBufferNormal, texCord).rgb;
    
        float AO = 0.0;
    
        // loop through the kernel
        for (int i = 0 ; i < MAX_KERNEL_SIZE ; i++) {

            vec3 randomVec = reflect(gKernel[i], normalize(randomAxis));
            
            // We transform the sphere kernel to hemisphere kernel by checking
            // the angle between the normal and the randomVector, if > 90 degree,
            // (hence dot product < 0), we just flip the vector.
            if (dot(randomVec, FragNormView) < 0)
            {
                randomVec = randomVec * -1.0;
            }
                        
            vec4 checkPoint = vec4(FragPosView + randomVec, 1.0); // generate a random point
            vec4 projectedCheckPoint = projectionMatrix * checkPoint; // clip space
            vec3 NDCCheckPoint = projectedCheckPoint.xyz / projectedCheckPoint.w; // NDC space
            NDCCheckPoint = NDCCheckPoint * 0.5 + 0.5;
            
            vec3 realGeometryPoint = texture(gBufferPosition, NDCCheckPoint.xy).rgb;
            
            float rangeCheck= abs(realGeometryPoint.z - checkPoint.z ) < 0.12 ? 1.0 : 0.0;

            AO += (realGeometryPoint.z >= checkPoint.z + 0.005 ? 1.0 : 0.0) * rangeCheck ;

        }

    outFragColor = 1.0 - AO/ 32;
    }
}