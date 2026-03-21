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
      _texture_target_state(1),
      _is_dragging(false),
      _picked_object(nullptr),
      _last_mouse_x(0.0),
      _last_mouse_y(0.0)
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

Vec3 App::getRayFromMouse(double mouse_x, double mouse_y) {
    float nx = (2.0f * (float)mouse_x) / (float)_config.window_width - 1.0f;
    float ny = 1.0f - (2.0f * (float)mouse_y) / (float)_config.window_height;

    Mat4 invProj = _scene.getProjectionMatrix().inverse();
    
    // In NDC, Z range is -1 to 1 (OpenGL). We pick a point on the far plane or just the direction.
    // We'll compute the ray direction in view space.
    float view_x = invProj[0][0] * nx + invProj[1][0] * ny + invProj[2][0] * (-1.0f) + invProj[3][0];
    float view_y = invProj[0][1] * nx + invProj[1][1] * ny + invProj[2][1] * (-1.0f) + invProj[3][1];
    float view_z = -1.0f; 

    // Transform direction to world space (using inverse View matrix)
    Mat4 invView = _scene.getCamera().getViewMatrix().inverse();
    float world_x = invView[0][0] * view_x + invView[1][0] * view_y + invView[2][0] * view_z;
    float world_y = invView[0][1] * view_x + invView[1][1] * view_y + invView[2][1] * view_z;
    float world_z = invView[0][2] * view_x + invView[1][2] * view_y + invView[2][2] * view_z;

    return Vec3(world_x, world_y, world_z).normalized();
}

void App::loadAssets() {
    IParser* parser = new ObjParser();
    std::map<std::string, std::shared_ptr<Texture>> loadedTextures;
    int currentGroupId = 0;

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
                object.groupId = currentGroupId;
                _scene.addObject(object);
            }
            currentGroupId++;

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

    // Mouse Drag to Move
    double cur_x, cur_y;
    _window.getMousePos(&cur_x, &cur_y);
    int cur_btn = _window.getMouseButton(GLFW_MOUSE_BUTTON_LEFT);

    if (cur_btn == GLFW_PRESS) {
        if (!_is_dragging) {
            _is_dragging = true;
            _picked_object = nullptr;
            
            Vec3 ray_dir = getRayFromMouse(cur_x, cur_y);
            Vec3 ray_origin = _scene.getCamera().pos;
            
            float min_dist = 1e9f;
            for (auto& obj : _scene.getObjects()) {
                float dist;
                if (obj.intersectsRay(ray_origin, ray_dir, dist)) {
                    if (dist < min_dist) {
                        min_dist = dist;
                        _picked_object = &obj;
                    }
                }
            }
        } else if (_picked_object) {
            float dx = static_cast<float>(cur_x - _last_mouse_x);
            float dy = static_cast<float>(cur_y - _last_mouse_y);

            float sensitivity = 0.01f; 

            Mat4 invView = _scene.getCamera().getViewMatrix().inverse();
            Vec3 right(invView[0][0], invView[0][1], invView[0][2]);
            Vec3 up(invView[1][0], invView[1][1], invView[1][2]);

            int pid = _picked_object->groupId;
            for (auto& obj : _scene.getObjects()) {
                if (obj.groupId == pid) {
                    obj.position += right * (dx * sensitivity) - up * (dy * sensitivity);
                    obj.velocity = Vec3(0, 0, 0); // Reset velocity while dragging
                }
            }
        }
    } else {
        _is_dragging = false;
        _picked_object = nullptr;
    }
    _last_mouse_x = cur_x;
    _last_mouse_y = cur_y;
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
