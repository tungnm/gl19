#version 460

in vec3 Position;
in vec3 Normal;

out vec4 FragColor;

uniform vec4 LightPosView;

void main()
{
    vec3 s = normalize(LightPosView.xyz - Position);
    
    float diffuseIntensity = dot(s,Normal);
    
    vec3 diffuse = diffuseIntensity * vec3(0.9f, 0.5f, 0.3f);
    
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(-Position.xyz);
    
    vec3 r = 2 * diffuseIntensity * Normal - s;
      
    float specularIntensity = pow(dot(r, v),3);
    
    vec3 specular = specularIntensity * vec3(0.7);
    
    FragColor = vec4(diffuse + specular, 1.0);

    
}