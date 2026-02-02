#pragma once

#include "Shader.hpp"
#include "Mat4.hpp"
#include <vector>
#include <string>

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void draw(const Mat4& view, const Mat4& projection);
    
private:
    unsigned int _vao, _vbo;
    unsigned int _cubemapTexture;
    Shader _skyboxShader;

    void setupMesh();
    static unsigned int loadCubemap(const std::vector<std::string>& faces);
};
