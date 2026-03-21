#pragma once
#include <vector>
#include <string>
#include "GameObject.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mat4.hpp"
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <limits.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Skybox.hpp"
#include <memory>

class MainScene
{
public:
    MainScene()
        : _mainShader(nullptr), _texture_mix_factor(1.0f), _fov(45.0f), _aspect_ratio(1.0f), _optical_mode(0) {
    }

    ~MainScene() = default;

    void init(int window_width, int window_height, const std::string& vertexPath, const std::string& fragmentPath) {
        _aspect_ratio = static_cast<float>(window_width) / static_cast<float>(window_height);
        _mainShader = std::unique_ptr<Shader>(new Shader(vertexPath, fragmentPath));
        updateProjectionMatrix();
    }

    void addObject(const GameObject &object) {
        _objects.push_back(object);
    }
    
    void setSkybox(const std::string& path) {
        char currentPath[PATH_MAX];
        if (getcwd(currentPath, sizeof(currentPath)) == nullptr) {
            throw std::runtime_error("Failed to get current working directory for skybox.");
        }
        std::string currentPathStr = std::string(currentPath);

        std::string projectRootPath = currentPathStr;
        std::string resource_suffix = "/resources";
        if (currentPathStr.length() >= resource_suffix.length() && 
            currentPathStr.substr(currentPathStr.length() - resource_suffix.length()) == resource_suffix) {
            projectRootPath = currentPathStr.substr(0, currentPathStr.length() - resource_suffix.length());
        }

        const std::vector<std::string> face_names = {"posx", "negx", "posy", "negy", "posz", "negz"};
        const std::vector<std::string> extensions = {".jpg", ".bmp", ".png"};
        std::vector<std::string> final_faces;

        for (const auto& face_name : face_names) {
            bool found = false;
            for (const auto& ext : extensions) {
                std::string full_path = path + "/" + face_name + ext;
                std::string absolutePath = projectRootPath + "/" + full_path;

                std::ifstream f(absolutePath.c_str());
                if (f.good()) {
                    final_faces.push_back(absolutePath);
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Skybox texture not found for face: " + path + "/" + face_name);
            }
        }
        _skybox.reset(new Skybox(final_faces));
    }

    void render() {
        if (!_mainShader) return;

        // 1. Render Skybox first
        if (_skybox) {
            _skybox->draw(_camera.getViewMatrix(), _projection);
        }

        // 2. Render Objects
        _mainShader->use();
        
        // --- Common Uniforms ---
        _mainShader->setVec3("u_LightDir", -1.0f, -1.0f, -1.0f);
        _mainShader->setVec3("u_LightColor", 1.0f, 1.0f, 1.0f);
        _mainShader->setVec3("u_ViewPos", _camera.pos.x, _camera.pos.y, _camera.pos.z);
        _mainShader->setFloat("u_TextureMixFactor", _texture_mix_factor);
        _mainShader->setInt("u_OpticalMode", _optical_mode);

        // --- Bind skybox for environment effects to unit 1 ---
        if (_skybox) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, _skybox->getTextureID());
            _mainShader->setInt("u_Skybox", 1);
        }

        Mat4 viewMatrix = _camera.getViewMatrix();
        
        for(auto& object: _objects){
            Mat4 transform = _projection * viewMatrix * object.model_matrix;
            _mainShader->setMat4("transform", transform.m);
            _mainShader->setMat4("model", object.model_matrix.m);

            // Pass material properties from .mtl
            _mainShader->setVec3("u_Material.ambient", object.material.ambient.x, object.material.ambient.y, object.material.ambient.z);
            _mainShader->setVec3("u_Material.diffuse", object.material.diffuse.x, object.material.diffuse.y, object.material.diffuse.z);
            _mainShader->setVec3("u_Material.specular", object.material.specular.x, object.material.specular.y, object.material.specular.z);
            _mainShader->setFloat("u_Material.shininess", object.material.shininess);
            _mainShader->setFloat("u_Material.opacity", object.material.opacity);

            object.texture->bind(0);
            _mainShader->setInt("u_Texture", 0);

            object.mesh->draw();
        }
    }

    void update(float delta_time) {
        for(auto& object: _objects){
            object.update(delta_time);
        }
    }

    Camera& getCamera() { return _camera; }
    std::vector<GameObject>& getObjects() { return _objects; }
    Mat4 getProjectionMatrix() const { return _projection; }
    void setTextureMixFactor(float factor) { _texture_mix_factor = factor; }
    float getTextureMixFactor() const { return _texture_mix_factor; }

    void setOpticalMode(int mode) { _optical_mode = mode; }
    int getOpticalMode() const { return _optical_mode; }

    void setFov(float newFov) {
        if (newFov < 1.0f) newFov = 1.0f;
        if (newFov > 90.0f) newFov = 90.0f;
        _fov = newFov;
        updateProjectionMatrix();
    }
    float getFov() const { return _fov; }

private:
    void updateProjectionMatrix() {
        _projection = Mat4().perspective(_fov * (M_PI / 180.0f), _aspect_ratio, 0.1f, 100.0f);
    }

    std::vector<GameObject> _objects;
    Mat4 _projection;
    std::unique_ptr<Shader> _mainShader;
    Camera _camera;
    float _texture_mix_factor;
    float _fov;
    float _aspect_ratio;
    std::unique_ptr<Skybox> _skybox;
    int _optical_mode; // 0: MTL Material, 1: Refraction
};
