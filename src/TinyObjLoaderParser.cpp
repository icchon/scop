#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "TinyObjLoaderParser.hpp"
#include <unordered_map>
#include <iostream>

ParsedData TinyObjLoaderParser::parse(const std::string& filepath) {
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.vertex_color = false;
    
    if (!reader.ParseFromFile(filepath, reader_config)) {
        throw std::runtime_error("TinyObjReader: " + reader.Error());
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    ParsedData data;
    std::unordered_map<std::string, GLuint> uniqueVertices;

    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            size_t fv = shape.mesh.num_face_vertices[f];
            
            int material_id = shape.mesh.material_ids[f];

            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t index = shape.mesh.indices[index_offset + v];

                std::string key = std::to_string(index.vertex_index) + "_" +
                                  std::to_string(index.texcoord_index) + "_" +
                                  std::to_string(material_id);

                if (uniqueVertices.count(key) == 0) {
                    uniqueVertices[key] = static_cast<GLuint>(data.vertices.size());
                    
                    Vertex vert;
                    vert.pos[0] = attrib.vertices[3 * index.vertex_index + 0];
                    vert.pos[1] = attrib.vertices[3 * index.vertex_index + 1];
                    vert.pos[2] = attrib.vertices[3 * index.vertex_index + 2];

                    if (!attrib.colors.empty() && index.vertex_index >= 0) {
                        vert.color[0] = attrib.colors[3 * index.vertex_index + 0];
                        vert.color[1] = attrib.colors[3 * index.vertex_index + 1];
                        vert.color[2] = attrib.colors[3 * index.vertex_index + 2];
                        std::cout << "Vertex Color found!" << std::endl;
                        std::cout << "R: " << vert.color[0]
                                  << " G: " << vert.color[1]
                                  << " B: " << vert.color[2] << std::endl;
                    } else if (material_id >= 0) {
                        vert.color[0] = materials[material_id].diffuse[0];
                        vert.color[1] = materials[material_id].diffuse[1];
                        vert.color[2] = materials[material_id].diffuse[2];
                    }
                    else {
                        vert.color[0] = 1.0f; vert.color[1] = 0.0f; vert.color[2] = 0.0f;
                    }
                    
                    if (index.texcoord_index >= 0) {
                        vert.texCoord[0] = attrib.texcoords[2 * index.texcoord_index + 0];
                        vert.texCoord[1] = attrib.texcoords[2 * index.texcoord_index + 1];
                    } else {
                        vert.texCoord[0] = 0.0f; vert.texCoord[1] = 0.0f;
                    }
                    
                    data.vertices.push_back(vert);
                }
                data.indices.push_back(uniqueVertices[key]);
            }
            index_offset += fv;
        }
    }
    return data;
}
