#include "graphics/common.hpp"
#include "graphics/window.hpp"

#include <stdexcept>

// TODO: save more general window specifications
Window::Window(int width, int height, const char* title) : width(width), height(height), title(title) {}

Window::~Window() {
    destroy();
}

void Window::create() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("[Window] Cannot create window");
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this); // save a pointer to this Window instance; to be accessed in callbacks

    if (!gladLoadGL(glfwGetProcAddress)) {
        throw std::runtime_error("[Window] Cannot initialize GLAD");
    }

    glViewport(0, 0, width, height);
}

void Window::update() {
    glfwSwapBuffers(window);
}

void Window::destroy() {
    if (window) {
        glfwDestroyWindow(window);
    }

    window = nullptr;
}

glm::ivec2 Window::size() const {
    return { width, height };
}

GLFWwindow* Window::ptr() const {
    return window;
}

bool Window::should_close() const {
    return glfwWindowShouldClose(window);
}
