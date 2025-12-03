#pragma once

#include "ecs/ecs.hpp"
#include "events/event_manager.hpp"
#include "graphics/window.hpp"
#include "systems/common.hpp"

#include <memory>
#include <queue>

struct RenderCall {
    glm::mat4 transform;
    GLuint VAO;
    GLuint shader_id;
    GLuint texture_unit;
    size_t n_vertices;
};

class ClientApplication {
public:
    ClientApplication(int width, int height);
    ~ClientApplication();
    ClientApplication(const ClientApplication&) = delete;
    ClientApplication(ClientApplication&&) = default;
    ClientApplication& operator=(const ClientApplication&) = delete;
    ClientApplication& operator=(ClientApplication&&) = default;

    // Start main loop of the application.
    void run();

    void update(float dt);
    void render();
    void stop();

private:
    int width;
    int height;
    std::unique_ptr<Window> window;
    bool should_close = false;
    bool cursor_disabled = true;

    EntityComponentSystem ECS{};
    std::shared_ptr<PhysicsSystem> physics_system{};
    std::vector<RenderCall> render_queue{};
    EventManager event_manager{};
    std::queue<Event> events{};

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
