#include "ObjParser.hpp"
#include "Vec3.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <cstdlib> // For rand()
#include <ctime>   // For time()

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
    // Seed random for face colors if needed
    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(NULL)));
        seeded = true;
    }

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
    // uniqueVertices is removed to ensure each face has its own vertices with its own color

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

            // Generate a color for this face
            GLfloat faceR = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
            GLfloat faceG = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
            GLfloat faceB = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);

            for (size_t i = 1; i < faceIndices.size() - 1; ++i) {
                ObjIndex triangle[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };
                
                // Calculate face normal
                Vec3 v0(positions[3 * triangle[0].v + 0], positions[3 * triangle[0].v + 1], positions[3 * triangle[0].v + 2]);
                Vec3 v1(positions[3 * triangle[1].v + 0], positions[3 * triangle[1].v + 1], positions[3 * triangle[1].v + 2]);
                Vec3 v2(positions[3 * triangle[2].v + 0], positions[3 * triangle[2].v + 1], positions[3 * triangle[2].v + 2]);
                Vec3 normal = cross(v1 - v0, v2 - v0).normalized();

                // Determine smooth color based on 6 orthogonal axes
                GLfloat nCol[3];
                float nx = normal.x;
                float ny = normal.y;
                float nz = normal.z;

                nCol[0] = std::max(0.0f, nx) + std::max(0.0f, -ny) + std::max(0.0f, -nz);
                nCol[1] = std::max(0.0f, -nx) + std::max(0.0f, ny) + std::max(0.0f, -nz);
                nCol[2] = std::max(0.0f, -nx) + std::max(0.0f, -ny) + std::max(0.0f, nz);

                // Normalize to ensure color components are within [0, 1]
                float maxCol = std::max({nCol[0], nCol[1], nCol[2], 1.0f});
                nCol[0] /= maxCol;
                nCol[1] /= maxCol;
                nCol[2] /= maxCol;

                for (int j = 0; j < 3; ++j) {
                    Vertex vert;
                    vert.pos[0] = positions[3 * triangle[j].v + 0];
                    vert.pos[1] = positions[3 * triangle[j].v + 1];
                    vert.pos[2] = positions[3 * triangle[j].v + 2];
                    
                    // Assign normal-based color
                    vert.normalColor[0] = nCol[0];
                    vert.normalColor[1] = nCol[1];
                    vert.normalColor[2] = nCol[2];

                    // Assign actual normal for lighting
                    vert.normal[0] = normal.x;
                    vert.normal[1] = normal.y;
                    vert.normal[2] = normal.z;

                    // If the file has vertex colors, use them, otherwise use the face color
                    if (colors.size() > (size_t)(3 * triangle[j].v + 2)) {
                        vert.color[0] = colors[3 * triangle[j].v + 0];
                        vert.color[1] = colors[3 * triangle[j].v + 1];
                        vert.color[2] = colors[3 * triangle[j].v + 2];
                    } else {
                        vert.color[0] = faceR; vert.color[1] = faceG; vert.color[2] = faceB;
                    }

                    if (triangle[j].vt >= 0 && texCoords.size() > (size_t)(2 * triangle[j].vt + 1)) {
                        vert.texCoord[0] = texCoords[2 * triangle[j].vt + 0];
                        vert.texCoord[1] = texCoords[2 * triangle[j].vt + 1];
                    } else {
                        vert.texCoord[0] = vert.pos[0];
                        vert.texCoord[1] = vert.pos[1];
                    }
                    data.indices.push_back(static_cast<GLuint>(data.vertices.size()));
                    data.vertices.push_back(vert);
                }
            }
        }
    }

    return data;
}
