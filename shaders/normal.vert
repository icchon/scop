#version 330 core

layout (location = 0) in vec3 aPos;     
layout (location = 1) in vec3 aColor;   
layout (location = 2) in vec3 aNormalColor;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in vec2 aTexCoord;

out vec3 vColor;
out vec3 vNormalColor;
out vec3 vNormal;
out vec2 vTexCoord;

uniform mat4 transform; // MVP Matrix
uniform mat4 model;     // Model Matrix

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);

    vColor = aColor;
    vNormalColor = aNormalColor;
    
    // Transform normal to world space (ignore translation)
    vNormal = mat3(model) * aNormal;
    
    vTexCoord = aTexCoord;
}
