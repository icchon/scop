
#pragma once
#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    GLuint ID;
    Shader(const std::string &vertexPath, const std::string &fragmentPath);
    ~Shader();
    void use() const;
    void setMat4(const std::string &name, const GLfloat *value) const;
    void setInt(const std::string &name, int value) const;
    void setBool(const std::string &name, bool value) const;
    void setFloat(const std::string &name, float value) const;
};
