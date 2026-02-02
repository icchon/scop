#version 330 core

in vec3 vColor; 
in vec2 vTexCoord;
out vec4 FinalColor;

uniform sampler2D u_Texture;
uniform float u_TextureMixFactor;

void main() {
    vec4 texColor = texture(u_Texture, vTexCoord) * vec4(vColor, 1.0);
    vec4 solidColor = vec4(vColor, 1.0);
    FinalColor = mix(solidColor, texColor, u_TextureMixFactor);
}
