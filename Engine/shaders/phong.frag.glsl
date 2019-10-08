#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCord;

out vec4 FragColor;

uniform vec4 LightPosView;
// texture sampler
uniform sampler2D Tex1;

void main()
{
    //sample texture
    vec3 texColor = texture(Tex1, TexCord).rgb;
    
    vec3 s = normalize(LightPosView.xyz - Position);
    
    float diffuseIntensity = dot(s,Normal);
    
    vec3 diffuse = diffuseIntensity * texColor;//vec3(0.9f, 0.5f, 0.3f);
    
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(-Position.xyz);
    
    //normal phong using reflection vector
    //vec3 r = 2 * diffuseIntensity * Normal - s;
    
    //blinn-phong use half way vector h
    vec3 h = normalize(v + s);
    
    // phong
    //float specularIntensity = pow(dot(r, v), 3);
    
    //blinn-phong
    float specularIntensity = pow(dot(h, Normal), 9);
    
    vec3 specular = specularIntensity * vec3(0.7);
    
    vec3 ambient = 0.05 * vec3(1, 1, 1);
    
    FragColor = vec4(ambient + diffuse + specular, 1.0);

    
}