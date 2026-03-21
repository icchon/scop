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
    Vec3 getRayFromMouse(double mouse_x, double mouse_y);

    AppConfig _config;
    Window _window;
    MainScene _scene;
    
    double _last_time;
    
    int _t_key_state;
    int _v_key_state;
    int _texture_target_state;

    bool _is_dragging;
    GameObject* _picked_object;
    double _last_mouse_x, _last_mouse_y;
};
