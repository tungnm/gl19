#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

uniform vec4 LightPosView;
uniform mat4 ModelView;
uniform mat3 normalToView;
uniform mat4 MVP;
uniform mat4 toLightSpace;

out vec3 Color;
out vec4 VertexPosLightSpace;

subroutine void RenderPassType();
subroutine uniform RenderPassType RenderPass;


subroutine (RenderPassType)
void shadeWithShadow() {
    //shadow map logic:
    // convert vertex position into light space
    VertexPosLightSpace = toLightSpace * VertexPosition;
    VertexPosLightSpace = VertexPosLightSpace * 0.5 + 0.5; 

    // transform normal to view space
    vec3 n = normalize(normalToView * VertexNormal);
    vec4 vertexPosInView = ModelView * vec4(VertexPosition, 1.0);
    vec3 s = normalize(vec3(LightPosView - vertexPosInView));
    
    float diffuseIntensity = max(0, dot(s,n));
    
    vec3 diffuse = diffuseIntensity * vec3(0.9f, 0.5f, 0.3f);
    
    // since in view space camera is at (0,0,0), the vector from
    // the vertex to the camera = (0,0,0) - vertex position in View
    vec3 v = normalize(-vertexPosInView.xyz);
    
    vec3 r = 2 * diffuseIntensity * n - s;
      
    float specularIntensity = pow(max(0,dot(r, v)),2);
    
    vec3 specular = specularIntensity * vec3(0.8);
    
    Color = diffuse + specular;
}

subroutine (RenderPassType)
void recordDepth() {
// Do nothing, depth will be written automatically
}

void main()
{
    
    
    RenderPass();
    gl_Position = MVP * vec4(VertexPosition,1.0);
    
    
    
}