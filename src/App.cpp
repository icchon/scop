#include "App.hpp"
#include "TinyObjLoaderParser.hpp"
#include "Camera.hpp"
#include <iostream>
#include <memory>
#include <map>

App::App()
    : _window(800, 600, "Scop"),
      _scene("shaders/normal.vert", "shaders/normal.frag"),
      _last_time(0.0),
      _t_key_state(GLFW_RELEASE),
      _texture_target_state(1)
{
    if (!_window.isOpen()) {
        throw std::runtime_error("Failed to create window");
    }
    loadAssets();
    _last_time = glfwGetTime();
}

App::~App() {
}

void App::loadAssets() {
    IParser* parser = new TinyObjLoaderParser();
    std::map<std::string, std::shared_ptr<Mesh>> loadedMeshes;
    std::map<std::string, std::shared_ptr<Texture>> loadedTextures;

    std::vector<ObjectConfig> configs = {
        { "objects/low_poly_tree/Lowpoly_tree_sample.obj", "textures/tree.png", Vec3(-1.5f, 0.0f, 0.0f), Vec3(0,1,0), 0.0f },
        // { "objects/85-cottage_obj/cottage_obj.obj", "objects/96_city/textures/download.jpg", Vec3(1.5f, -0.4f, 0.0f), Vec3(0,1,0), -90.0f },
        // { "objects/bugatti/bugatti.obj", "objects/96_city/textures/download.jpg", Vec3(0.0f, -0.4f, -3.0f), Vec3(0,1,0), 45.0f }
    };

    for (const auto& config : configs) {
        try {
            if (loadedMeshes.find(config.objPath) == loadedMeshes.end()) {
                ParsedData data = parser->parse(config.objPath);
                auto mesh = std::make_shared<Mesh>(data.vertices, data.indices);
                mesh->normalize();
                loadedMeshes[config.objPath] = mesh;
            }

            if (loadedTextures.find(config.texturePath) == loadedTextures.end()) {
                std::cout << "Loading texture: '" << config.texturePath << "'" << std::endl;
                loadedTextures[config.texturePath] = std::make_shared<Texture>(config.texturePath);
            }

            Mat4 model_matrix;
            model_matrix = model_matrix.translate(config.position);
            if (config.rotation_angle_deg != 0.0f) {
                 model_matrix = model_matrix.rotate(config.rotation_angle_deg * (M_PI / 180.0f), config.rotation_axis);
            }

            _scene.addObject(GameObject(
                loadedMeshes[config.objPath],
                loadedTextures[config.texturePath],
                model_matrix
            ));

        } catch(const std::exception& e) {
            std::cerr << "Error loading object " << config.objPath << ": " << e.what() << std::endl;
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
        _scene.getCamera().move(CameraDirection::FORWARD, CAMERA_SPEED * delta_time);
    if(_window.getKey(GLFW_KEY_S) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::BACKWARD, CAMERA_SPEED * delta_time);
    if(_window.getKey(GLFW_KEY_A) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::LEFT, CAMERA_SPEED * delta_time);
    if(_window.getKey(GLFW_KEY_D) == GLFW_PRESS)
        _scene.getCamera().move(CameraDirection::RIGHT, CAMERA_SPEED * delta_time);

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
