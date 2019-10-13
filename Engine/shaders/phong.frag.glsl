#version 460

in vec3 sTanSpace;
in vec3 vTanSpace;
in vec2 TexCord;

out vec4 FragColor;

// texture sampler
uniform sampler2D Tex1;
uniform sampler2D normalMapTex;
uniform sampler2D heightMapTex;


const float bumpScale = 0.03;

vec2 findOffset(vec3 v, out float height) {
    const int nSteps = int(mix(60, 10, abs(v.z)));
    float htStep = 1.0 / nSteps;
    vec2 deltaT = (v.xy * bumpScale) / (nSteps * v.z);
    float ht = 1.0;
    vec2 tc = TexCord.xy;
    height = texture(heightMapTex, tc).r;
    while( height < ht ) {
        ht -= htStep;
        tc -= deltaT;
        height = texture(heightMapTex, tc).r;
    }
    return tc;
}


bool isOccluded(float height, vec2 tc, vec3 s) {
    // Shadow ray cast
    const int nShadowSteps = int(mix(60,10,s.z));
    float htStep = 1.0 / nShadowSteps;
    vec2 deltaT = (s.xy * bumpScale) / ( nShadowSteps * s.z );
    float ht = height + htStep * 0.5;
    while( height < ht && ht < 1.0 ) {
        ht += htStep;
        tc += deltaT;
        height = texture(heightMapTex, tc).r;
    }
    return ht < 1.0;
}

void main()
{
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(vTanSpace);
    vec3 s = normalize(sTanSpace);
    
    float height = 1.0;
    vec2 tc = findOffset(v, height);
    
    vec3 newN = texture(normalMapTex, tc).xyz;
    newN.xy = 2.0 * newN.xy - 1.0;
    newN = normalize(newN);


    //sample texture
    vec3 texColor = texture(Tex1, tc).rgb;

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