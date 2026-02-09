#pragma once
#include <vector>
#include <string>
#include "GameObject.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Mat4.hpp"
#include <fstream>    // for std::ifstream
#include <iostream>   // for std::cerr
#include <unistd.h>   // for getcwd
#include <limits.h>   // for PATH_MAX

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "Skybox.hpp"
#include <memory>

class MainScene
{
public:
    MainScene()
        : _mainShader(nullptr), _texture_mix_factor(1.0f), _fov(45.0f), _aspect_ratio(1.0f) {
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
                std::string full_path = path + "/" + face_name + ext; // relative path from config
                std::string absolutePath = projectRootPath + "/" + full_path; // combined absolute path

                std::ifstream f(absolutePath.c_str());
                if (f.good()) {
                    final_faces.push_back(absolutePath); // Absolute path for Skybox constructor
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

        _mainShader->use();
        _mainShader->setInt("u_Texture", 0);
        _mainShader->setFloat("u_TextureMixFactor", _texture_mix_factor);
        
        Mat4 viewMatrix = _camera.getViewMatrix();
        
        for(auto& object: _objects){
            Mat4 transform = _projection * viewMatrix * object.model_matrix;
            _mainShader->setMat4("transform", transform.m);
            object.texture->bind();
            object.mesh->draw();
        }

        if (_skybox) {
            _skybox->draw(_camera.getViewMatrix(), _projection);
        }
    }

    void update(float delta_time) {
        (void)delta_time;
        // Rotation logic removed to be handled elsewhere if needed
    }

    Camera& getCamera() { return _camera; }
    void setTextureMixFactor(float factor) { _texture_mix_factor = factor; }
    float getTextureMixFactor() const { return _texture_mix_factor; }

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
};
