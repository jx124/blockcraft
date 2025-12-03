#pragma once

#include "graphics/common.hpp"

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

    int width;
    int height;
private:
    const char* title;
    GLFWwindow* window{};
};
