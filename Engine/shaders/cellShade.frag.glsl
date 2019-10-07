#version 460

in vec3 Position;
in vec3 Normal;

out vec4 FragColor;

uniform vec4 LightPosView;

void main()
{
    vec3 s = normalize(LightPosView.xyz - Position);
    
    float diffuseIntensity = floor(dot(s,Normal) * 3) / 3.0;
    
    vec3 diffuse = diffuseIntensity * vec3(0.9f, 0.5f, 0.3f);
    
    vec3 ambient = 0.05 * vec3(1, 1, 1);
    
    FragColor = vec4(ambient + diffuse, 1.0);

    
}