#pragma once
#include "Mesh.hpp"

struct ParsedData {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

class IParser {
public:
    virtual ~IParser() = default;
    virtual ParsedData parse(const std::string& filePath) = 0;
};
