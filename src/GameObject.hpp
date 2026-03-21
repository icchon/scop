#pragma once
#include <memory>
#include <string>
#include <algorithm>
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Mat4.hpp"
#include "Vec3.hpp"
#include "IParser.hpp" 

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GameObject {
public:
    GameObject(std::shared_ptr<Mesh> m, std::shared_ptr<Texture> t, 
               Vec3 pos, Vec3 rot, float s)
        : mesh(m), texture(t), position(pos), initial_rotation(rot), scale(s), auto_rotation_y(0.0f) {
        calculateBoundingBox();
        updateModelMatrix();
    };
    ~GameObject() = default;

    Vec3 min_local, max_local;

    void calculateBoundingBox() {
        if (mesh && !mesh->getVertices().empty()) {
            const auto& verts = mesh->getVertices();
            min_local = Vec3(verts[0].pos[0], verts[0].pos[1], verts[0].pos[2]);
            max_local = min_local;
            for (const auto& v : verts) {
                min_local.x = std::min(min_local.x, v.pos[0]);
                min_local.y = std::min(min_local.y, v.pos[1]);
                min_local.z = std::min(min_local.z, v.pos[2]);
                max_local.x = std::max(max_local.x, v.pos[0]);
                max_local.y = std::max(max_local.y, v.pos[1]);
                max_local.z = std::max(max_local.z, v.pos[2]);
            }
        }
    }

    bool intersectsRay(const Vec3& ray_origin, const Vec3& ray_dir, float& distance) {
        // Simple AABB intersection in world space (roughly scaled)
        Vec3 world_min = position + min_local * scale;
        Vec3 world_max = position + max_local * scale;

        float tmin = (world_min.x - ray_origin.x) / ray_dir.x;
        float tmax = (world_max.x - ray_origin.x) / ray_dir.x;
        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (world_min.y - ray_origin.y) / ray_dir.y;
        float tymax = (world_max.y - ray_origin.y) / ray_dir.y;
        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax)) return false;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (world_min.z - ray_origin.z) / ray_dir.z;
        float tzmax = (world_max.z - ray_origin.z) / ray_dir.z;
        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) return false;
        if (tzmin > tmin) tmin = tzmin;
        if (tzmax < tmax) tmax = tzmax;

        distance = tmin;
        return tmin > 0;
    }

    void update(float delta_time) {
        auto_rotation_y -= 30.0f * delta_time;
        if (auto_rotation_y < -360.0f) auto_rotation_y += 360.0f;
        updateModelMatrix();
    }

    void updateModelMatrix() {
        Mat4 m;
        m = m.translate(position);
        m = m.rotate(auto_rotation_y * (M_PI / 180.0f), Vec3(0, 1, 0));
        if (initial_rotation.z != 0.0f) m = m.rotate(initial_rotation.z * (M_PI / 180.0f), Vec3(0, 0, 1));
        if (initial_rotation.y != 0.0f) m = m.rotate(initial_rotation.y * (M_PI / 180.0f), Vec3(0, 1, 0));
        if (initial_rotation.x != 0.0f) m = m.rotate(initial_rotation.x * (M_PI / 180.0f), Vec3(1, 0, 0));
        m = m.scale(scale);
        model_matrix = m;
    }

    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture> texture;
    Mat4 model_matrix;
    
    int groupId;
    Vec3 position;
    Vec3 initial_rotation;
    float scale;
    float auto_rotation_y;

    Material material;
};
