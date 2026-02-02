#pragma once
#include <string>
#include <GL/glew.h>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(GLuint slot = 0) const;
    void unbind() const;

private:
    GLuint _ID;
    std::string _filePath;
    unsigned char* _localBuffer;
    int _width, _height, _BPP;
};
