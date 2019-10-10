#version 460

in vec3 sTanSpace;
in vec3 vTanSpace;
in vec2 TexCord;

out vec4 FragColor;

// texture sampler
uniform sampler2D Tex1;
uniform sampler2D normalMapTex;

void main()
{
    //sample texture
    vec3 texColor = texture(Tex1, TexCord).rgb;
    //sample normal map
    vec3 nTanSpace = texture(normalMapTex, TexCord).xyz;
    nTanSpace = 2.0 * nTanSpace - 1.0;

    vec3 s = normalize(sTanSpace);
    float diffuseIntensity = dot(s, nTanSpace);
    
    vec3 diffuse = diffuseIntensity * texColor;//vec3(0.9f, 0.5f, 0.3f);
    
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(vTanSpace);
    
    //blinn-phong use half way vector h
    vec3 h = normalize(v + s);
    
    //blinn-phong
    float specularIntensity = pow(dot(h, nTanSpace), 4);
    
    vec3 specular = specularIntensity * vec3(0.8);
    
    vec3 ambient = 0.05 * texColor;
    
    FragColor = vec4(ambient + diffuse + specular, 1.0);

    
}