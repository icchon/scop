#pragma once
#include <GL/glew.h>
#include <vector>

struct Vertex {
    GLfloat pos[3];
    GLfloat color[3];
    GLfloat texCoord[2];
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices);
    ~Mesh();
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    void draw() const;
    void normalize();
    void updateBuffers();   
private:
    GLuint _VAO, _VBO, _EBO;
    std::vector<Vertex> _vertices;
    std::vector<GLuint> _indices;
};
