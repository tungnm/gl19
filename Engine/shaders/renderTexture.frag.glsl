#version 460

in vec2 texCord;

out vec4 outFragColor;

layout (binding=0) uniform sampler2D textureSampler;

void main()
{
    vec3 color = texture(textureSampler, texCord).rgb;
    outFragColor = vec4(color, 1.0);
}