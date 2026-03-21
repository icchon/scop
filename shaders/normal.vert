#version 330 core

layout (location = 0) in vec3 aPos;     
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoord;

uniform mat4 transform; // MVP Matrix
uniform mat4 model;     // Model Matrix

void main()
{
    gl_Position = transform * vec4(aPos, 1.0);
    vFragPos = vec3(model * vec4(aPos, 1.0));
    vNormal = mat3(model) * aNormal;
    vTexCoord = aTexCoord;
}
