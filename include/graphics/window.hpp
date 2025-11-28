#pragma once

#include "graphics/common.hpp"
#include "graphics/camera.hpp"

struct WindowState {
    bool first_mouse = true;
    int last_x;
    int last_y;

    bool go_forward = false;
    bool go_backward = false;
    bool go_left = false;
    bool go_right = false;
};

class Window {
public:
    Window(int width, int height, const char* title);

    GLFWwindow* ptr() const;
    Camera camera{};
    WindowState state{};

private:
    int width;
    int height;
    const char* title;
    GLFWwindow* window {};

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
