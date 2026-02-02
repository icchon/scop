#pragma once
#include <vector>
#include <string>
#include "GameObject.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mat4.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Skybox.hpp"
#include <memory>

class MainScene
{
public:
    MainScene(const std::string& vertexPath, const std::string& fragmentPath)
        : _mainShader(vertexPath, fragmentPath), _texture_mix_factor(1.0f) {
        
        _fov = 45.0f;
        float aspect_ratio = 800.0f / 600.0f;
        _projection = Mat4().perspective(_fov * (M_PI / 180.0f), aspect_ratio, 0.1f, 100.0f);

        std::vector<std::string> faces = {
            "textures/field-skyboxes/Sorsele3/posx.jpg", // 右 (Positive X)
            "textures/field-skyboxes/Sorsele3/negx.jpg", // 左 (Negative X)
            "textures/field-skyboxes/Sorsele3/posy.jpg", // 上 (Positive Y)
            "textures/field-skyboxes/Sorsele3/negy.jpg", // 下 (Negative Y)
            "textures/field-skyboxes/Sorsele3/posz.jpg", // 奥 (Positive Z)
            "textures/field-skyboxes/Sorsele3/negz.jpg"  // 手前 (Negative Z)
        };
        std::vector<std::string> faces2 = {
            "textures/DaylightBoxPieces/posx.bmp", // 右 (Positive X)
            "textures/DaylightBoxPieces/negx.bmp", // 左 (Negative X)
            "textures/DaylightBoxPieces/posy.bmp", // 上 (Positive Y)
            "textures/DaylightBoxPieces/negy.bmp", // 下 (Negative Y)
            "textures/DaylightBoxPieces/posz.bmp", // 奥 (Positive Z)
            "textures/DaylightBoxPieces/negz.bmp"  // 手前 (Negative Z)
        };
        (void)faces; // 未使用変数警告を防ぐ
        _skybox.reset(new Skybox(faces2));
    }

    ~MainScene() = default;

    void addObject(const GameObject &object) {
        _objects.push_back(object);
    }

    void render() {
        _mainShader.use();
        _mainShader.setInt("u_Texture", 0);
        _mainShader.setFloat("u_TextureMixFactor", _texture_mix_factor);
        
        Mat4 viewMatrix = _camera.getViewMatrix();
        
        // Draw regular objects
        for(auto& object: _objects){
            Mat4 transform = _projection * viewMatrix * object.model_matrix;
            _mainShader.setMat4("transform", transform.m);
            object.texture->bind();
            object.mesh->draw();
        }

        // Draw skybox as last
        _skybox->draw(_camera.getViewMatrix(), _projection);
    }

    void update(float delta_time) {
        float rotation_speed = 0.5f; // radians per second
        for (auto& object : _objects) {
            object.model_matrix = object.model_matrix.rotate(rotation_speed * delta_time, Vec3(0, 1, 0));
        }
    }

    Camera& getCamera() { return _camera; }
    void setTextureMixFactor(float factor) { _texture_mix_factor = factor; }
    float getTextureMixFactor() const { return _texture_mix_factor; }

    // FOV制御のためのメソッド
    void setFov(float newFov) {
        // FOVの範囲を制限する (例: 1.0f から 90.0f)
        if (newFov < 1.0f) newFov = 1.0f;
        if (newFov > 90.0f) newFov = 90.0f;
        _fov = newFov;
        float aspect_ratio = 800.0f / 600.0f; // TODO: Windowサイズから取得するようにする
        _projection = Mat4().perspective(_fov * (M_PI / 180.0f), aspect_ratio, 0.1f, 100.0f);
    }
    float getFov() const { return _fov; }


private:
    std::vector<GameObject> _objects;
    Mat4 _projection;
    Shader _mainShader;
    Camera _camera;
    float _texture_mix_factor;
    float _fov; // FOVをメンバー変数として追加
    std::unique_ptr<Skybox> _skybox;
};
