#pragma once
#include "Mesh.hpp"
#include "Vec3.hpp"
#include <string>
#include <vector>
#include <algorithm>

struct Material {
    Vec3 ambient;     
    Vec3 diffuse;     
    Vec3 specular;    
    float shininess;  
    float opacity;    
    float ior;        
    bool has_texture;
    std::string texture_path;

    Material() : ambient(0.2f, 0.2f, 0.2f), diffuse(0.7f, 0.7f, 0.7f), 
                 specular(0.3f, 0.3f, 0.3f), shininess(32.0f), 
                 opacity(1.0f), ior(1.0f), has_texture(false) {}
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    Material material;
};

struct ParsedData {
    std::vector<MeshData> meshes;

    void normalize() {
        if (meshes.empty()) return;

        float minX, minY, minZ;
        float maxX, maxY, maxZ;
        bool first = true;

        // 1. Find global bounding box
        for (const auto& mesh : meshes) {
            for (const auto& v : mesh.vertices) {
                if (first) {
                    minX = maxX = v.pos[0];
                    minY = maxY = v.pos[1];
                    minZ = maxZ = v.pos[2];
                    first = false;
                } else {
                    minX = std::min(minX, v.pos[0]);
                    maxX = std::max(maxX, v.pos[0]);
                    minY = std::min(minY, v.pos[1]);
                    maxY = std::max(maxY, v.pos[1]);
                    minZ = std::min(minZ, v.pos[2]);
                    maxZ = std::max(maxZ, v.pos[2]);
                }
            }
        }

        if (first) return;

        float centerX = (minX + maxX) / 2.0f;
        float centerY = (minY + maxY) / 2.0f;
        float centerZ = (minZ + maxZ) / 2.0f;

        float width  = maxX - minX;
        float height = maxY - minY;
        float depth  = maxZ - minZ;
        float maxDim = std::max({width, height, depth});
        float scale = (maxDim != 0) ? (2.0f / maxDim) : 1.0f;

        // 2. Apply same transformation to ALL vertices in ALL meshes
        for (auto& mesh : meshes) {
            for (auto& v : mesh.vertices) {
                v.pos[0] = (v.pos[0] - centerX) * scale;
                v.pos[1] = (v.pos[1] - centerY) * scale;
                v.pos[2] = (v.pos[2] - centerZ) * scale;
            }
        }
    }
};

class IParser {
public:
    virtual ~IParser() = default;
    virtual ParsedData parse(const std::string& filePath) = 0;
};
