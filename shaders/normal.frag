#version 330 core

in vec3 vColor; 
in vec3 vNormalColor;
in vec3 vNormal;
in vec2 vTexCoord;
out vec4 FinalColor;

uniform sampler2D u_Texture;
uniform float u_TextureMixFactor;
uniform int u_ColorMode; // 0: random, 1: normal-based
uniform vec3 u_LightDir; // e.g., vec3(-1.0, -1.0, -1.0)
uniform vec3 u_LightColor; // e.g., vec3(1.0, 1.0, 1.0)

void main() {
    vec4 texColor = texture(u_Texture, vTexCoord);
    vec3 baseCol = (u_ColorMode == 0) ? vColor : vNormalColor;
    
    // Simple Lambertian lighting
    // ambient
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * u_LightColor;
    
    // diffuse 
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(-u_LightDir); // direction from surface to light
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;
    
    vec4 solidColor = vec4(baseCol, 1.0);
    vec3 lighting = ambient + diffuse;
    
    FinalColor = vec4(mix(solidColor.rgb, texColor.rgb, u_TextureMixFactor) * lighting, 1.0);
}
