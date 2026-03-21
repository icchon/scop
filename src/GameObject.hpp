#pragma once
#include <memory>
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Mat4.hpp"
#include <cmath>

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
        // 時計回りに自転 (30度/秒)
        auto_rotation_y -= 30.0f * delta_time;
        if (auto_rotation_y < -360.0f) auto_rotation_y += 360.0f;
        updateModelMatrix();
    }

    void updateModelMatrix() {
        Mat4 m;
        // 1. 位置を決める
        m = m.translate(position);
        
        // 2. 垂直軸(Y軸)周りの自転を適用
        // これにより、オブジェクトが初期状態で傾いていても、世界座標の「上」を向いた軸で回転する
        m = m.rotate(auto_rotation_y * (M_PI / 180.0f), Vec3(0, 1, 0));

        // 3. 設定ファイルからの初期回転を適用
        if (initial_rotation.z != 0.0f)
             m = m.rotate(initial_rotation.z * (M_PI / 180.0f), Vec3(0, 0, 1));
        if (initial_rotation.y != 0.0f)
             m = m.rotate(initial_rotation.y * (M_PI / 180.0f), Vec3(0, 1, 0));
        if (initial_rotation.x != 0.0f)
             m = m.rotate(initial_rotation.x * (M_PI / 180.0f), Vec3(1, 0, 0));

        // 4. スケールを適用
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
};
