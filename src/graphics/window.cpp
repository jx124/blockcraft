#include "graphics/common.hpp"
#include "graphics/window.hpp"

#include <iostream>
#include <stdexcept>

Window::Window(int width, int height, const char* title) : width(width), height(height), title(title) {
    if (!glfwInit()) {
        throw std::runtime_error("[Window] Cannot initialize GLFW");
    }
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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    std::cout << "[OpenGL] Version: " << glGetString(GL_VERSION) << std::endl;
    glDebugMessageCallback(error_message_callback, nullptr);
#endif

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

GLFWwindow* Window::ptr() const {
    return window;
}

void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Window* win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));
    win_ptr->width = width;
    win_ptr->height= height;

    glViewport(0, 0, width, height);
}

void Window::error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
        GLsizei length, const GLchar* message, const void* userParam) {

    (void) id, (void) length, (void) userParam;
    const char* source_str;
    const char* type_str;
    const char* severity_str;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            source_str = "OpenGL API call";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            source_str = "Window-system API call";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            source_str = "Shader compilation";
            break;
        default:
            source_str = "Other";
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            type_str = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            type_str = "Deprecated behavior";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            type_str = "Undefined behavior";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            type_str = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            type_str = "Performance";
            break;
        default:
            type_str = "Other";
    }

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severity_str = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severity_str = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severity_str = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severity_str = "Notification";
            break;
    }

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    std::fprintf(stderr, "[OpenGL] %s: %s.\n\tSeverity: %s.\n\tMessage: %s\n",
            type_str, source_str, severity_str, message);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)window, (void)scancode, (void)mods;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    std::cout << "Key: " << key << ", action: " <<
        (action == GLFW_PRESS 
            ? "press" 
            : action == GLFW_RELEASE
            ? "release"
            : "repeat") << std::endl;
}

void Window::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    (void)window, (void)xpos, (void)ypos;
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window, (void)button, (void)action, (void)mods;
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window, (void)xoffset, (void)yoffset;
}
