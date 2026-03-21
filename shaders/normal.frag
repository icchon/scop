#version 330 core

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;
out vec4 FinalColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float opacity;
};

uniform Material u_Material;
uniform sampler2D u_Texture;
uniform samplerCube u_Skybox;
uniform float u_TextureMixFactor;
uniform int u_OpticalMode; // 0: Material, 1: Glass
uniform vec3 u_LightDir; 
uniform vec3 u_LightColor;
uniform vec3 u_ViewPos;

void main() {
    vec4 texColor = texture(u_Texture, vTexCoord);
    
    // Setup vectors
    vec3 norm = normalize(vNormal);
    if (length(vNormal) < 0.01) norm = normalize(u_ViewPos - vFragPos);

    vec3 lightDir = normalize(-u_LightDir); 
    vec3 viewToCam = normalize(u_ViewPos - vFragPos); 
    vec3 viewToPos = normalize(vFragPos - u_ViewPos);
    vec3 halfDir = normalize(lightDir + viewToCam);

    // 1. Common Lighting Elements
    float diff = max(dot(norm, lightDir), 0.0);
    float sh = max(u_Material.shininess, 1.0);
    float spec = pow(max(dot(norm, halfDir), 0.0), sh);

    vec3 finalRGB;
    float finalAlpha = u_Material.opacity;

    if (u_OpticalMode == 0) {
        // --- Material Mode (MTL colors + Phong) ---
        vec3 baseColor = mix(u_Material.diffuse, texColor.rgb, u_TextureMixFactor);
        
        // Polished look: If texture is hidden, enhance specular and add subtle reflection
        float effectFactor = 1.0 - u_TextureMixFactor;
        
        vec3 ambient = (u_Material.ambient + vec3(0.1)) * u_LightColor;
        vec3 diffuse = diff * baseColor * u_LightColor;
        vec3 specular = spec * u_Material.specular * u_LightColor;
        
        // Add subtle environment reflection when texture is hidden
        vec3 reflectDir = reflect(viewToPos, norm);
        vec3 envReflect = texture(u_Skybox, reflectDir).rgb;
        float fresnel = pow(1.0 - max(dot(viewToCam, norm), 0.0), 3.0);
        float reflectionStrength = 0.15 * effectFactor * fresnel;
        
        finalRGB = mix(ambient + diffuse + specular, envReflect, reflectionStrength);
    } 
    else {
        // --- Glass Mode (Refraction + Dispersion) ---
        float kR = 0.65; float kG = 0.67; float kB = 0.69;
        vec3 envColor;
        envColor.r = texture(u_Skybox, refract(viewToPos, norm, kR)).r;
        envColor.g = texture(u_Skybox, refract(viewToPos, norm, kG)).g;
        envColor.b = texture(u_Skybox, refract(viewToPos, norm, kB)).b;
        
        // Rim light for glass
        float rimFactor = pow(1.0 - max(dot(viewToCam, norm), 0.0), 3.0);
        vec3 rimLight = rimFactor * vec3(0.6, 0.8, 1.0);
        
        vec3 glassBase = mix(envColor, texColor.rgb, u_TextureMixFactor);
        finalRGB = glassBase + (spec * u_LightColor * 0.5) + rimLight;
        finalAlpha = mix(0.3, 1.0, u_TextureMixFactor);
    }

    FinalColor = vec4(finalRGB, finalAlpha);
}
