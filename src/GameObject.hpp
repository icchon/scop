#pragma once
#include <memory>
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Mat4.hpp"


class GameObject {
public:
    GameObject(std::shared_ptr<Mesh> m, std::shared_ptr<Texture> t, const Mat4& model,
               const Vec3& rot_axis = Vec3(0.0f, 0.0f, 0.0f), float rot_speed = 0.0f)
        : mesh(m), texture(t), model_matrix(model), rotation_axis(rot_axis), rotation_speed_deg_per_sec(rot_speed) {};
    ~GameObject() = default;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Texture> texture;
    Mat4 model_matrix;
    Vec3 rotation_axis;
    float rotation_speed_deg_per_sec;
};
