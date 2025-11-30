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
    ~Window();
    Window(const Window&) = delete;
    Window(Window&&) = default;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = default;

    void create();
    void update();
    void destroy();

    glm::ivec2 size() const;
    GLFWwindow* ptr() const;
    bool should_close() const;

    // TODO: remove from Window class
    Camera camera{};
    WindowState state{};
    int width;
    int height;
private:
    const char* title;
    GLFWwindow* window{};
};
