#version 460

uniform vec4 LightPosView;
in vec2 texCord;

out vec4 outFragColor;

// texture sampler: read from textures of G-buffer
// layout (binding=0) means set the texture unit 0 for this sampler2D gBufferPosition
layout (binding=0) uniform sampler2D gBufferPosition;
layout (binding=1) uniform sampler2D gBufferNormal;
layout (binding=2) uniform sampler2D gBufferColor;

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
        float specularIntensity = pow(max(0, dot(h, FragNormView)), 8);
            
        specular = specularIntensity * vec3(0.8);
    }
    
    vec3 ambient = 0.05 * FragColor;
    outFragColor = vec4(ambient + diffuse + specular, 1.0);

}