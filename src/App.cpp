#include "App.hpp"
#include "TinyObjLoaderParser.hpp"
#include "Camera.hpp"
#include <iostream>
#include <memory>
#include <map>

App::App(const AppConfig& config)
    : _config(config),
      _window(config.window_width, config.window_height, config.window_title.c_str()),
      _scene(),
      _last_time(0.0),
      _t_key_state(GLFW_RELEASE),
      _texture_target_state(1)
{
    if (!_window.isOpen()) {
        throw std::runtime_error("Failed to create window");
    }
    // Initialize scene with shaders and projection matrix
    _scene.init(_config.window_width, _config.window_height, 
                _config.shader_vertex_path, _config.shader_fragment_path);

    loadAssets();
    _last_time = glfwGetTime();
}

App::~App() {
}

void App::loadAssets() {
    IParser* parser = new TinyObjLoaderParser();
    std::map<std::string, std::shared_ptr<Mesh>> loadedMeshes;
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
            if (loadedMeshes.find(obj_config.path) == loadedMeshes.end()) {
                ParsedData data = parser->parse(obj_config.path);
                auto mesh = std::make_shared<Mesh>(data.vertices, data.indices);
                mesh->normalize();
                loadedMeshes[obj_config.path] = mesh;
            }

            if (loadedTextures.find(obj_config.texture) == loadedTextures.end()) {
                std::cout << "Loading texture: '" << obj_config.texture << "'" << std::endl;
                loadedTextures[obj_config.texture] = std::make_shared<Texture>(obj_config.texture);
            }

            Mat4 model_matrix;
            model_matrix = model_matrix.translate(obj_config.position);
            
            if (obj_config.rotation.z != 0.0f)
                 model_matrix = model_matrix.rotate(obj_config.rotation.z * (M_PI / 180.0f), Vec3(0, 0, 1));
            if (obj_config.rotation.y != 0.0f)
                 model_matrix = model_matrix.rotate(obj_config.rotation.y * (M_PI / 180.0f), Vec3(0, 1, 0));
            if (obj_config.rotation.x != 0.0f)
                 model_matrix = model_matrix.rotate(obj_config.rotation.x * (M_PI / 180.0f), Vec3(1, 0, 0));

            model_matrix = model_matrix.scale(obj_config.scale);

            _scene.addObject(GameObject(
                loadedMeshes[obj_config.path],
                loadedTextures[obj_config.texture],
                model_matrix
            ));

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
        yaw_offset -= ROTATION_SPEED * delta_time;
    if(_window.getKey(GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw_offset += ROTATION_SPEED * delta_time;
    if(_window.getKey(GLFW_KEY_UP) == GLFW_PRESS)
        pitch_offset += ROTATION_SPEED * delta_time;
    if(_window.getKey(GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch_offset -= ROTATION_SPEED * delta_time;
    
    if (yaw_offset != 0.0f || pitch_offset != 0.0f) {
        _scene.getCamera().rotate(yaw_offset, pitch_offset);
    }

    // FOV制御
    float current_fov = _scene.getFov();
    if (_window.getKey(GLFW_KEY_Z) == GLFW_PRESS) {
        _scene.setFov(current_fov - FOV_CHANGE_SPEED * delta_time);
    }
    if (_window.getKey(GLFW_KEY_X) == GLFW_PRESS) {
        _scene.setFov(current_fov + FOV_CHANGE_SPEED * delta_time);
    }

    if(_window.getKey(GLFW_KEY_T) == GLFW_PRESS && _t_key_state == GLFW_RELEASE) {
        _texture_target_state = 1 - _texture_target_state;
    }
    _t_key_state = _window.getKey(GLFW_KEY_T);
}

void App::update(float delta_time) {
    _scene.update(delta_time);
    float current_mix_factor = _scene.getTextureMixFactor();
    if (_texture_target_state == 1 && current_mix_factor < 1.0f) {
        current_mix_factor += TRANSITION_SPEED * delta_time;
        if (current_mix_factor > 1.0f) current_mix_factor = 1.0f;
        _scene.setTextureMixFactor(current_mix_factor);
    } else if (_texture_target_state == 0 && current_mix_factor > 0.0f) {
        current_mix_factor -= TRANSITION_SPEED * delta_time;
        if (current_mix_factor < 0.0f) current_mix_factor = 0.0f;
        _scene.setTextureMixFactor(current_mix_factor);
    }
}

void App::render() {
    _window.clear(0.5f, 0.5f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    _scene.render();
    _window.swapBuffers();
}
