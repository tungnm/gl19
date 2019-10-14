#version 460

in vec3 sTanSpace;
in vec3 vTanSpace;
in vec2 TexCord;

out vec4 FragColor;

// texture sampler
uniform sampler2D DiffuseTexture;
uniform sampler2D NormalMapTexture;

void main()
{
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(vTanSpace);
    vec3 s = normalize(sTanSpace);
        
    vec3 newN = texture(NormalMapTexture, TexCord).xyz;
    newN.xy = 2.0 * newN.xy - 1.0;
    newN = normalize(newN);

    //sample texture
    vec3 texColor = texture(DiffuseTexture, TexCord).rgb;

    float diffuseIntensity = dot(s, newN);
        
    vec3 diffuse = vec3(0,0,0);
    vec3 specular = vec3(0,0,0);
    
    diffuse = diffuseIntensity * texColor;//vec3(0.9f, 0.5f, 0.3f);
        
    //blinn-phong use half way vector h
    vec3 h = normalize(v + s);
        
    //blinn-phong
    float specularIntensity = pow(dot(h, newN), 4);
        
    specular = specularIntensity * vec3(0.8);
        
    vec3 ambient = 0.05 * texColor;
    
    FragColor = vec4(ambient + diffuse + specular, 1.0);

    
}