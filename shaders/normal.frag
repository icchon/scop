#version 330 core

in vec3 vColor; 
in vec3 vNormalColor;
in vec2 vTexCoord;
out vec4 FinalColor;

uniform sampler2D u_Texture;
uniform float u_TextureMixFactor;
uniform int u_ColorMode; // 0: random, 1: normal-based

void main() {
    vec4 texColor = texture(u_Texture, vTexCoord);
    vec3 baseCol = (u_ColorMode == 0) ? vColor : vNormalColor;
    vec4 solidColor = vec4(baseCol, 1.0);
    FinalColor = mix(solidColor, texColor, u_TextureMixFactor);
}
