#pragma once
#include <memory>
#include <string>
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Mat4.hpp"
#include "Vec3.hpp"
#include "IParser.hpp" // For Material definition

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class GameObject {
public:
    GameObject(std::shared_ptr<Mesh> m, std::shared_ptr<Texture> t, 
               Vec3 pos, Vec3 rot, float s)
        : mesh(m), texture(t), position(pos), initial_rotation(rot), scale(s), auto_rotation_y(0.0f) {
        updateModelMatrix();
    };
    ~GameObject() = default;

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
    
    Vec3 position;
    Vec3 initial_rotation;
    float scale;
    float auto_rotation_y;

    Material material;
};
