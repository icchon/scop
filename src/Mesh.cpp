#include "Mesh.hpp"


Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices): _vertices(vertices), _indices(indices) {
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glGenBuffers(1, &_EBO);

    updateBuffers();
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_VBO);
    glDeleteBuffers(1, &_EBO);
}

void Mesh::draw() const {
    glBindVertexArray(_VAO);
    glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::normalize() {
    if (_vertices.empty()) return;

    float minX, minY, minZ;
    float maxX, maxY, maxZ;

    minX = maxX = _vertices[0].pos[0];
    minY = maxY = _vertices[0].pos[1];
    minZ = maxZ = _vertices[0].pos[2];

    for (const auto& v : _vertices) {
        if (v.pos[0] < minX) minX = v.pos[0];
        if (v.pos[0] > maxX) maxX = v.pos[0];
        if (v.pos[1] < minY) minY = v.pos[1];
        if (v.pos[1] > maxY) maxY = v.pos[1];
        if (v.pos[2] < minZ) minZ = v.pos[2];
        if (v.pos[2] > maxZ) maxZ = v.pos[2];
    }
    
    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;

    float width  = maxX - minX;
    float height = maxY - minY;
    float depth  = maxZ - minZ;
    float maxDim = std::max({width, height, depth});
    
    float scale = (maxDim != 0) ? (2.0f / maxDim) : 1.0f;

    for (auto& v : _vertices) {
        v.pos[0] = (v.pos[0] - centerX) * scale;
        v.pos[1] = (v.pos[1] - centerY) * scale;
        v.pos[2] = (v.pos[2] - centerZ) * scale;
    }

    updateBuffers();
}

void Mesh::updateBuffers(){   
    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(Vertex), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(GLuint), _indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}
