#include "ObjParser.hpp"
#include "Vec3.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unistd.h>
#include <limits.h>
#include <algorithm>
#include <map>

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

static std::map<std::string, Material> parseMtl(const std::string& filepath) {
    std::map<std::string, Material> materials;
    std::ifstream file(filepath);
    if (!file.is_open()) return materials;

    std::string line, currentMtlName;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        if (!(ss >> prefix)) continue;

        if (prefix == "newmtl") {
            ss >> currentMtlName;
            materials[currentMtlName] = Material();
        } else if (!currentMtlName.empty()) {
            if (prefix == "Ka") ss >> materials[currentMtlName].ambient.x >> materials[currentMtlName].ambient.y >> materials[currentMtlName].ambient.z;
            else if (prefix == "Kd") ss >> materials[currentMtlName].diffuse.x >> materials[currentMtlName].diffuse.y >> materials[currentMtlName].diffuse.z;
            else if (prefix == "Ks") ss >> materials[currentMtlName].specular.x >> materials[currentMtlName].specular.y >> materials[currentMtlName].specular.z;
            else if (prefix == "Ns") ss >> materials[currentMtlName].shininess;
            else if (prefix == "d" || prefix == "Tr") ss >> materials[currentMtlName].opacity;
            else if (prefix == "map_Kd") {
                ss >> materials[currentMtlName].texture_path;
                materials[currentMtlName].has_texture = true;
            }
        }
    }
    return materials;
}

ParsedData ObjParser::parse(const std::string& filepath) {
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) throw std::runtime_error("getcwd failed");
    std::string projectRootPath = std::string(currentPath);
    if (projectRootPath.length() >= 10 && projectRootPath.substr(projectRootPath.length() - 10) == "/resources")
        projectRootPath = projectRootPath.substr(0, projectRootPath.length() - 10);

    std::string absoluteFilepath = projectRootPath + "/" + filepath;
    std::string baseDir = absoluteFilepath.substr(0, absoluteFilepath.find_last_of("/\\") + 1);

    std::ifstream file(absoluteFilepath);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + absoluteFilepath);

    std::vector<GLfloat> positions, texCoords, normals;
    std::map<std::string, Material> materialLib;
    std::map<std::string, size_t> meshMap;
    ParsedData data;

    std::string line, currentMtlName = "default";
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        if (!(ss >> prefix)) continue;

        if (prefix == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;
            materialLib = parseMtl(baseDir + mtlFile);
        } else if (prefix == "usemtl") {
            ss >> currentMtlName;
        } else if (prefix == "v") {
            float x, y, z; ss >> x >> y >> z;
            positions.push_back(x); positions.push_back(y); positions.push_back(z);
        } else if (prefix == "vt") {
            float u, v; ss >> u >> v;
            texCoords.push_back(u); texCoords.push_back(v);
        } else if (prefix == "vn") {
            float x, y, z; ss >> x >> y >> z;
            normals.push_back(x); normals.push_back(y); normals.push_back(z);
        } else if (prefix == "f") {
            if (meshMap.find(currentMtlName) == meshMap.end()) {
                meshMap[currentMtlName] = data.meshes.size();
                MeshData m;
                if (materialLib.count(currentMtlName)) m.material = materialLib[currentMtlName];
                data.meshes.push_back(m);
            }
            auto& currentMesh = data.meshes[meshMap[currentMtlName]];
            
            std::string part;
            std::vector<ObjIndex> fIdx;
            while (ss >> part) {
                int v=0, vt=0, vn=0;
                size_t s1 = part.find('/'), s2 = std::string::npos;
                if (s1 != std::string::npos) s2 = part.find('/', s1 + 1);
                v = std::stoi(part.substr(0, s1));
                if (s1 != std::string::npos && s2 != s1 + 1) vt = std::stoi(part.substr(s1+1, s2-s1-1));
                if (s2 != std::string::npos) vn = std::stoi(part.substr(s2+1));
                
                v = (v > 0) ? v - 1 : (v < 0) ? (positions.size()/3) + v : 0;
                vt = (vt > 0) ? vt - 1 : (vt < 0) ? (texCoords.size()/2) + vt : -1;
                vn = (vn > 0) ? vn - 1 : (vn < 0) ? (normals.size()/3) + vn : -1;
                fIdx.push_back({v, vt, vn});
            }

            // Triangulate
            for (size_t i = 1; i < fIdx.size() - 1; ++i) {
                ObjIndex tri[3] = { fIdx[0], fIdx[i], fIdx[i+1] };
                Vec3 faceN(0,0,0);
                if (tri[0].vn < 0) {
                    Vec3 v0(positions[3*tri[0].v], positions[3*tri[0].v+1], positions[3*tri[0].v+2]);
                    Vec3 v1(positions[3*tri[1].v], positions[3*tri[1].v+1], positions[3*tri[1].v+2]);
                    Vec3 v2(positions[3*tri[2].v], positions[3*tri[2].v+1], positions[3*tri[2].v+2]);
                    faceN = cross(v1-v0, v2-v0).normalized();
                }
                for (int j=0; j<3; ++j) {
                    Vertex vert;
                    vert.pos[0] = positions[3*tri[j].v]; vert.pos[1] = positions[3*tri[j].v+1]; vert.pos[2] = positions[3*tri[j].v+2];
                    if (tri[j].vn >= 0) {
                        vert.normal[0] = normals[3*tri[j].vn]; vert.normal[1] = normals[3*tri[j].vn+1]; vert.normal[2] = normals[3*tri[j].vn+2];
                    } else {
                        vert.normal[0] = faceN.x; vert.normal[1] = faceN.y; vert.normal[2] = faceN.z;
                    }
                    if (tri[j].vt >= 0) {
                        vert.texCoord[0] = texCoords[2*tri[j].vt]; vert.texCoord[1] = texCoords[2*tri[j].vt+1];
                    } else {
                        vert.texCoord[0] = vert.pos[0]; vert.texCoord[1] = vert.pos[1];
                    }
                    currentMesh.indices.push_back(static_cast<GLuint>(currentMesh.vertices.size()));
                    currentMesh.vertices.push_back(vert);
                }
            }
        }
    }
    if (data.meshes.empty()) data.meshes.push_back(MeshData()); // Fallback
    return data;
}
