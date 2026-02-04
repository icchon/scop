#pragma once

#include "Window.hpp"
#include "MainScene.hpp"
#include "Vec3.hpp"
#include "Config.hpp"
#include <string>
#include <vector>

class App {
public:
    App(const AppConfig& config);
    ~App();
    void run();

private:
    void processInput(float delta_time);
    void update(float delta_time);
    void render();
    void loadAssets();

    AppConfig _config;
    Window _window;
    MainScene _scene;
    
    double _last_time;
    
    const float ROTATION_SPEED = 40.0f;
    const float TRANSITION_SPEED = 2.0f;
    const float FOV_CHANGE_SPEED = 30.0f;
    int _t_key_state;
    int _texture_target_state;
};
