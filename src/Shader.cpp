
#include "Shader.hpp"
#include <vector>

static std::string shaderPathToString(const std::string &shaderPath);
static GLuint compileShader(const std::string &shaderRaw, GLenum type);

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath)
{
    std::string vertexCode = shaderPathToString(vertexPath);
    std::string fragmentCode = shaderPathToString(fragmentPath);

    GLuint vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);
    if (vertexShader == 0 || fragmentShader == 0)
    {
        this->ID = 0;
        return;
    }
    this->ID = glCreateProgram();
    glAttachShader(this->ID, vertexShader);
    glAttachShader(this->ID, fragmentShader);
    glLinkProgram(this->ID);

    int success;
    glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(this->ID, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
    glDeleteProgram(this->ID);
}

static std::string shaderPathToString(const std::string &shaderPath)
{
    std::ifstream fs(shaderPath);
    if (!fs.is_open())
    {
        std::cerr << "Failed to open shader file: " << shaderPath << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << fs.rdbuf();
    return ss.str();
}

static GLuint compileShader(const std::string &shaderRaw, GLenum type)
{
    GLuint id = glCreateShader(type);
    const char *str = shaderRaw.c_str();
    glShaderSource(id, 1, &str, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> errorLog(length);
        glGetShaderInfoLog(id, length, &length, &errorLog[0]);
        std::cerr << "Shader compilation failed: " << &errorLog[0] << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setMat4(const std::string& name, const GLfloat *value) const
{
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
}

void Shader::setInt(const std::string& name, int value) const
{
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    glUniform1i(location, value);
}

void Shader::setBool(const std::string& name, bool value) const
{
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    glUniform1i(location, (int)value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        std::cerr << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    glUniform1f(location, value);
}
