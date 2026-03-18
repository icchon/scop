#include "ObjParser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>
#include <limits.h>
#include <algorithm>

struct ObjIndex {
    int v, vt, vn;
    bool operator==(const ObjIndex& other) const {
        return v == other.v && vt == other.vt && vn == other.vn;
    }
};

struct ObjIndexHasher {
    size_t operator()(const ObjIndex& idx) const {
        size_t h1 = std::hash<int>{}(idx.v);
        size_t h2 = std::hash<int>{}(idx.vt);
        size_t h3 = std::hash<int>{}(idx.vn);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

ParsedData ObjParser::parse(const std::string& filepath) {
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        throw std::runtime_error("Failed to get current working directory for obj parsing.");
    }
    std::string currentPathStr = std::string(currentPath);

    std::string projectRootPath = currentPathStr;
    std::string resource_suffix = "/resources";
    if (currentPathStr.length() >= resource_suffix.length() && 
        currentPathStr.substr(currentPathStr.length() - resource_suffix.length()) == resource_suffix) {
        projectRootPath = currentPathStr.substr(0, currentPathStr.length() - resource_suffix.length());
    }

    std::string absoluteFilepath = projectRootPath + "/" + filepath;

    std::ifstream file(absoluteFilepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + absoluteFilepath);
    }

    std::vector<GLfloat> positions;
    std::vector<GLfloat> texCoords;
    std::vector<GLfloat> colors;

    ParsedData data;
    std::unordered_map<ObjIndex, GLuint, ObjIndexHasher> uniqueVertices;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        if (!(ss >> prefix)) continue;

        if (prefix == "v") {
            GLfloat x, y, z;
            ss >> x >> y >> z;
            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
            
            GLfloat r, g, b;
            if (ss >> r >> g >> b) {
                colors.push_back(r);
                colors.push_back(g);
                colors.push_back(b);
            }
        } else if (prefix == "vt") {
            GLfloat u, v;
            ss >> u >> v;
            texCoords.push_back(u);
            texCoords.push_back(v);
        } else if (prefix == "f") {
            std::string vertexPart;
            std::vector<ObjIndex> faceIndices;
            while (ss >> vertexPart) {
                int v = 0, vt = 0, vn = 0;
                size_t firstSlash = vertexPart.find('/');
                
                if (firstSlash == std::string::npos) {
                    v = std::stoi(vertexPart);
                } else {
                    v = std::stoi(vertexPart.substr(0, firstSlash));
                    size_t secondSlash = vertexPart.find('/', firstSlash + 1);
                    if (secondSlash == std::string::npos) {
                        // v/vt
                        vt = std::stoi(vertexPart.substr(firstSlash + 1));
                    } else {
                        // v/vt/vn or v//vn
                        if (secondSlash > firstSlash + 1) {
                            vt = std::stoi(vertexPart.substr(firstSlash + 1, secondSlash - firstSlash - 1));
                        }
                        if (secondSlash + 1 < vertexPart.size()) {
                            vn = std::stoi(vertexPart.substr(secondSlash + 1));
                        }
                    }
                }
                
                // Adjust for 1-based indexing
                v = (v > 0) ? v - 1 : (v < 0) ? (positions.size() / 3) + v : 0;
                vt = (vt > 0) ? vt - 1 : (vt < 0) ? (texCoords.size() / 2) + vt : -1;
                vn = (vn > 0) ? vn - 1 : (vn < 0) ? 0 : -1;

                faceIndices.push_back({v, vt, vn});
            }

            for (size_t i = 1; i < faceIndices.size() - 1; ++i) {
                ObjIndex triangle[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };
                for (int j = 0; j < 3; ++j) {
                    if (uniqueVertices.find(triangle[j]) == uniqueVertices.end()) {
                        uniqueVertices[triangle[j]] = static_cast<GLuint>(data.vertices.size());
                        Vertex vert;
                        vert.pos[0] = positions[3 * triangle[j].v + 0];
                        vert.pos[1] = positions[3 * triangle[j].v + 1];
                        vert.pos[2] = positions[3 * triangle[j].v + 2];
                        
                        if (colors.size() > (size_t)(3 * triangle[j].v + 2)) {
                            vert.color[0] = colors[3 * triangle[j].v + 0];
                            vert.color[1] = colors[3 * triangle[j].v + 1];
                            vert.color[2] = colors[3 * triangle[j].v + 2];
                        } else {
                            vert.color[0] = 0.5f; vert.color[1] = 0.5f; vert.color[2] = 0.5f; // Gray default
                        }

                        if (triangle[j].vt >= 0 && texCoords.size() > (size_t)(2 * triangle[j].vt + 1)) {
                            vert.texCoord[0] = texCoords[2 * triangle[j].vt + 0];
                            vert.texCoord[1] = texCoords[2 * triangle[j].vt + 1];
                        } else {
                            // Simple planar projection as fallback
                            vert.texCoord[0] = vert.pos[0];
                            vert.texCoord[1] = vert.pos[1];
                        }
                        data.vertices.push_back(vert);
                    }
                    data.indices.push_back(uniqueVertices[triangle[j]]);
                }
            }
        }
    }

    return data;
}
