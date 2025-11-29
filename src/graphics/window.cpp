#include "graphics/common.hpp"
#include "graphics/window.hpp"

#include "utils/logger.hpp"

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(error_message_callback, nullptr);
    log_debug("OpenGL Version %s", glGetString(GL_VERSION));
#endif
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

    log_error("OpenGL %s: %s.\n\tSeverity: %s.\n\tMessage: %s\n",
            type_str, source_str, severity_str, message);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode, (void)mods;

    Window* win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    log_debug("key: %d, action: %s", key,
        action == GLFW_PRESS 
            ? "press" 
            : action == GLFW_RELEASE
            ? "release"
            : "repeat");

    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        win_ptr->state.go_forward = true;
    }
    if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
        win_ptr->state.go_forward = false;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        win_ptr->state.go_backward = true;
    }
    if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
        win_ptr->state.go_backward = false;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        win_ptr->state.go_left = true;
    }
    if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
        win_ptr->state.go_left = false;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        win_ptr->state.go_right = true;
    }
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        win_ptr->state.go_right = false;
    }
}

void Window::cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    Window* win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (win_ptr->state.first_mouse) {
        win_ptr->state.last_x = x_pos;
        win_ptr->state.last_y = y_pos;
        win_ptr->state.first_mouse = false;
    }

    float yaw_offset = win_ptr->state.last_x - x_pos;
    float pitch_offset = win_ptr->state.last_y - y_pos;

    win_ptr->state.last_x = x_pos;
    win_ptr->state.last_y = y_pos;

    Camera& camera = win_ptr->camera;
    float sensitivity = camera.camera_sensitivity;

    yaw_offset *= sensitivity;
    pitch_offset *= sensitivity;

    float &yaw = camera.yaw;
    float &pitch = camera.pitch;

    yaw -= yaw_offset;
    pitch += pitch_offset;

    pitch = std::clamp(pitch, -89.9f, 89.9f);

    // pitch around x-axis first, then yaw around z-axis
    glm::vec3 direction(std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
            std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch)),
            std::sin(glm::radians(pitch)));
    camera.camera_front = glm::normalize(direction);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window, (void)button, (void)action, (void)mods;
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window, (void)xoffset, (void)yoffset;
}
