#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "TinyObjLoaderParser.hpp"
#include "Vec3.hpp"
#include <unordered_map>
#include <iostream>
#include <unistd.h> 
#include <limits.h> 

struct TinyObjVertexKey {
    int v, vn, vt;
    bool operator==(const TinyObjVertexKey& other) const {
        return v == other.v && vn == other.vn && vt == other.vt;
    }
};

struct TinyObjVertexHasher {
    size_t operator()(const TinyObjVertexKey& k) const {
        size_t h1 = std::hash<int>{}(k.v);
        size_t h2 = std::hash<int>{}(k.vn);
        size_t h3 = std::hash<int>{}(k.vt);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

ParsedData TinyObjLoaderParser::parse(const std::string& filepath) {
    char currentPath[PATH_MAX];
    if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
        throw std::runtime_error("Failed to get current working directory for obj parsing.");
    }
    std::string projectRootPath = std::string(currentPath);
    std::string resource_suffix = "/resources";
    if (projectRootPath.length() >= resource_suffix.length() && 
        projectRootPath.substr(projectRootPath.length() - resource_suffix.length()) == resource_suffix) {
        projectRootPath = projectRootPath.substr(0, projectRootPath.length() - resource_suffix.length());
    }

    std::string absoluteFilepath = projectRootPath + "/" + filepath;
    std::string baseDir = absoluteFilepath.substr(0, absoluteFilepath.find_last_of("/\\") + 1);

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = baseDir; 
    
    if (!reader.ParseFromFile(absoluteFilepath, reader_config)) {
        throw std::runtime_error("TinyObjReader: " + reader.Error());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    ParsedData data;
    std::unordered_map<int, size_t> materialToMeshIndex;
    std::vector<std::unordered_map<TinyObjVertexKey, GLuint, TinyObjVertexHasher>> meshUniqueVertices;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];
            int material_id = shape.mesh.material_ids[f];

            if (materialToMeshIndex.find(material_id) == materialToMeshIndex.end()) {
                materialToMeshIndex[material_id] = data.meshes.size();
                MeshData newMesh;
                if (material_id >= 0 && material_id < (int)materials.size()) {
                    const auto& m = materials[material_id];
                    newMesh.material.ambient = Vec3(m.ambient[0], m.ambient[1], m.ambient[2]);
                    newMesh.material.diffuse = Vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
                    newMesh.material.specular = Vec3(m.specular[0], m.specular[1], m.specular[2]);
                    newMesh.material.shininess = m.shininess;
                    newMesh.material.opacity = m.dissolve;
                    newMesh.material.ior = m.ior;
                    if (!m.diffuse_texname.empty()) {
                        newMesh.material.texture_path = m.diffuse_texname;
                        newMesh.material.has_texture = true;
                    }
                }
                data.meshes.push_back(newMesh);
                meshUniqueVertices.emplace_back();
            }

            size_t meshIdx = materialToMeshIndex[material_id];
            auto& currentMesh = data.meshes[meshIdx];
            auto& uniqueVertices = meshUniqueVertices[meshIdx];

            // Calculate face normal if needed
            Vec3 faceNormal(0, 0, 0);
            if (fv >= 3) {
                tinyobj::index_t idx0 = shape.mesh.indices[index_offset + 0];
                tinyobj::index_t idx1 = shape.mesh.indices[index_offset + 1];
                tinyobj::index_t idx2 = shape.mesh.indices[index_offset + 2];
                Vec3 v0(attrib.vertices[3 * idx0.vertex_index + 0], attrib.vertices[3 * idx0.vertex_index + 1], attrib.vertices[3 * idx0.vertex_index + 2]);
                Vec3 v1(attrib.vertices[3 * idx1.vertex_index + 0], attrib.vertices[3 * idx1.vertex_index + 1], attrib.vertices[3 * idx1.vertex_index + 2]);
                Vec3 v2(attrib.vertices[3 * idx2.vertex_index + 0], attrib.vertices[3 * idx2.vertex_index + 1], attrib.vertices[3 * idx2.vertex_index + 2]);
                faceNormal = cross(v1 - v0, v2 - v0).normalized();
            }

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t index = shape.mesh.indices[index_offset + v];
                TinyObjVertexKey key = {index.vertex_index, index.normal_index, index.texcoord_index};

                if (uniqueVertices.find(key) == uniqueVertices.end()) {
                    uniqueVertices[key] = static_cast<GLuint>(currentMesh.vertices.size());
                    Vertex vert;
                    
                    vert.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
                    vert.pos[1] = attrib.vertices[3 * index.vertex_index + 1];
                    vert.pos[2] = attrib.vertices[3 * index.vertex_index + 2];

                    if (index.normal_index >= 0) {
                        vert.normal[0] = attrib.normals[3 * index.normal_index + 0];
                        vert.normal[1] = attrib.normals[3 * index.normal_index + 1];
                        vert.normal[2] = attrib.normals[3 * index.normal_index + 2];
                    } else {
                        vert.normal[0] = faceNormal.x;
                        vert.normal[1] = faceNormal.y;
                        vert.normal[2] = faceNormal.z;
                    }

                    if (index.texcoord_index >= 0) {
                        vert.texCoord[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                        vert.texCoord[1] = attrib.texcoords[2 * index.texcoord_index + 1];
                    } else {
                        vert.texCoord[0] = vert.pos[0];
                        vert.texCoord[1] = vert.pos[1];
                    }

                    currentMesh.vertices.push_back(vert);
                }
                currentMesh.indices.push_back(uniqueVertices[key]);
            }
            index_offset += fv;
        }
    }

    if (data.meshes.empty()) {
        data.meshes.push_back(MeshData());
    }

    return data;
}
