#pragma once
#include <GL/glew.h>
#include <string>
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    bool isOpen() const;
    void pollEvents() const;
    void swapBuffers() const;
    int getKey(int key) const;
    void clear(float r, float g, float b, float a) const;
    
private:
    GLFWwindow* _window;
    void keyCallback(int key, int scancode, int action, int mods);  
};
