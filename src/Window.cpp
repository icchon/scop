#include "Window.hpp"
#include <iostream>

void Window::keyCallback(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(_window, GL_TRUE);
    }
    (void)scancode;
    (void)mods;
}

Window::Window(int width, int height, const std::string &title)
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        _window = nullptr;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    _window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!_window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(_window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwDestroyWindow(_window);
        glfwTerminate();
        _window = nullptr;
        return;
    }
    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win) {
            win->keyCallback(key, scancode, action, mods);
        } });
    glEnable(GL_DEPTH_TEST);
}

Window::~Window()
{
    if (_window)
    {
        glfwDestroyWindow(_window);
        glfwTerminate();
    }
}

bool Window::isOpen() const
{
    return _window && !glfwWindowShouldClose(_window);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

void Window::swapBuffers() const
{
    if (_window)
    {
        glfwSwapBuffers(_window);
    }
}

int Window::getKey(int key) const
{
    if (_window)
    {
        return glfwGetKey(_window, key);
    }
    return -1;
}

void Window::clear(float r, float g, float b, float a) const
{
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
