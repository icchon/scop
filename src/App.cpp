#include "App.hpp"
#include "ObjParser.hpp"
#include "Camera.hpp"
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>

App::App(const AppConfig& config)
    : _config(config),
      _window(config.window_width, config.window_height, config.window_title.c_str()),
      _scene(),
      _last_time(0.0),
      _t_key_state(GLFW_RELEASE),
      _v_key_state(GLFW_RELEASE),
      _texture_target_state(1)
{
    if (!_window.isOpen()) {
        throw std::runtime_error("Failed to create window");
    }
    _scene.init(_config.window_width, _config.window_height, 
                _config.shader_vertex_path, _config.shader_fragment_path);

    loadAssets();
    _last_time = glfwGetTime();
}

App::~App() {
}

void App::loadAssets() {
    IParser* parser = new ObjParser();
    std::map<std::string, std::shared_ptr<Texture>> loadedTextures;

    if (!_config.skybox_path.empty()) {
        try {
            _scene.setSkybox(_config.skybox_path);
        } catch (const std::exception& e) {
            std::cerr << "Error loading skybox: " << e.what() << std::endl;
        }
    }

    for (const auto& obj_config : _config.objects) {
        try {
            ParsedData data = parser->parse(obj_config.path);
            data.normalize();

            std::string objDir = "";
            size_t lastSlash = obj_config.path.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                objDir = obj_config.path.substr(0, lastSlash + 1);
            }

            for (const auto& mData : data.meshes) {
                auto mesh = std::make_shared<Mesh>(mData.vertices, mData.indices);

                std::string texToLoad = obj_config.texture;
                if (mData.material.has_texture) {
                    std::string mtlTexPath = mData.material.texture_path;
                    for (auto& c : mtlTexPath) if (c == '\\') c = '/';
                    texToLoad = objDir + mtlTexPath;
                }

                if (loadedTextures.find(texToLoad) == loadedTextures.end()) {
                    try {
                        std::cout << "Loading texture: '" << texToLoad << "'" << std::endl;
                        loadedTextures[texToLoad] = std::make_shared<Texture>(texToLoad);
                    } catch (...) {
                        std::cerr << "Failed to load texture: " << texToLoad << ", fallback." << std::endl;
                        texToLoad = obj_config.texture;
                        if (loadedTextures.find(texToLoad) == loadedTextures.end()) {
                            loadedTextures[texToLoad] = std::make_shared<Texture>(texToLoad);
                        }
                    }
                }

                GameObject object(
                    mesh,
                    loadedTextures[texToLoad],
                    obj_config.position,
                    obj_config.rotation,
                    obj_config.scale
                );
                object.material = mData.material;
                _scene.addObject(object);
            }

        } catch(const std::exception& e) {
            std::cerr << "Error loading object " << obj_config.path << ": " << e.what() << std::endl;
        }
    }
    delete parser;
}

void App::run() {
    while(_window.isOpen()) {
        double current_time = glfwGetTime();
        float delta_time = static_cast<float>(current_time - _last_time);
        _last_time = current_time;

        processInput(delta_time);
        update(delta_time);
        render();
    }
}

void App::processInput(float delta_time) {
    _window.pollEvents();

    if(_window.getKey(GLFW_KEY_W) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::FORWARD, _config.camera_speed * delta_time);
    if(_window.getKey(GLFW_KEY_S) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::BACKWARD, _config.camera_speed * delta_time);
    if(_window.getKey(GLFW_KEY_A) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::LEFT, _config.camera_speed * delta_time);
    if(_window.getKey(GLFW_KEY_D) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::RIGHT, _config.camera_speed * delta_time);

    float yaw_offset = 0.0f;
    float pitch_offset = 0.0f;
    if(_window.getKey(GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw_offset -= 40.0f * delta_time;
    if(_window.getKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw_offset += 40.0f * delta_time;
    if(_window.getKey(GLFW_KEY_UP) == GLFW_PRESS)
        pitch_offset += 40.0f * delta_time;
    if(_window.getKey(GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch_offset -= 40.0f * delta_time;
    
    if (yaw_offset != 0.0f || pitch_offset != 0.0f) {
        _scene.getCamera().rotate(yaw_offset, pitch_offset);
    }

    if(_window.getKey(GLFW_KEY_T) == GLFW_PRESS && _t_key_state == GLFW_RELEASE) {
        _texture_target_state = 1 - _texture_target_state;
    }
    _t_key_state = _window.getKey(GLFW_KEY_T);

    if(_window.getKey(GLFW_KEY_V) == GLFW_PRESS && _v_key_state == GLFW_RELEASE) {
        _scene.setOpticalMode(1 - _scene.getOpticalMode());
    }
    _v_key_state = _window.getKey(GLFW_KEY_V);
}

void App::update(float delta_time) {
    _scene.update(delta_time);
    float current_mix_factor = _scene.getTextureMixFactor();
    if (_texture_target_state == 1 && current_mix_factor < 1.0f) {
        current_mix_factor += 2.0f * delta_time;
        if (current_mix_factor > 1.0f) current_mix_factor = 1.0f;
        _scene.setTextureMixFactor(current_mix_factor);
    } else if (_texture_target_state == 0 && current_mix_factor > 0.0f) {
        current_mix_factor -= 2.0f * delta_time;
        if (current_mix_factor < 0.0f) current_mix_factor = 0.0f;
        _scene.setTextureMixFactor(current_mix_factor);
    }
}

void App::render() {
    _window.clear(0.15f, 0.15f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    _scene.render();
    _window.swapBuffers();
}
