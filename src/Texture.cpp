#include "Texture.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const std::string& path)
    : _ID(0), _filePath(path), _localBuffer(nullptr), _width(0), _height(0), _BPP(0) {
    
    stbi_set_flip_vertically_on_load(1);
    _localBuffer = stbi_load(path.c_str(), &_width, &_height, &_BPP, 4);

    glGenTextures(1, &_ID);
    glBindTexture(GL_TEXTURE_2D, _ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (_localBuffer) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, _localBuffer);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(_localBuffer);
    } else {
        std::cerr << "Error: Failed to load texture '" << path << "'" << std::endl;
    }

    glBindTexture(GL_TEXTURE_2D, 0); 
}

Texture::~Texture() {
    glDeleteTextures(1, &_ID);
}

void Texture::bind(GLuint slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, _ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
