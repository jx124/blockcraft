#pragma once

#include "ecs/ecs.hpp"
#include "events/event_manager.hpp"
#include "graphics/window.hpp"
#include "shadow_map.hpp"
#include "systems/common.hpp"
#include "networking/client.hpp"
#include "networking/server.hpp"

#include <cstdint>
#include <memory>
#include <queue>
#include <string>

constexpr float BLOCKS_PER_SECOND = 5.0f;
constexpr int SHADOW_WIDTH = 2048;
constexpr int SHADOW_HEIGHT = 2048;

class ClientApplication {
public:
    ClientApplication(int width, int height, std::string hostname, uint16_t port);
    ~ClientApplication();
    ClientApplication(const ClientApplication&) = delete;
    ClientApplication(ClientApplication&&) noexcept = delete;
    ClientApplication& operator=(const ClientApplication&) = delete;
    ClientApplication& operator=(ClientApplication&&) noexcept = delete;

    // Start main loop of the application.
    void run();

    void update();
    void render();
    void stop();

private:
    int width;
    int height;
    std::string hostname;
    uint16_t port;
    std::unique_ptr<Window> window;
    bool should_close = false;
    bool cursor_disabled = true;
    float previous_time = 0.0f;
    float dt = 0.0f;
    float frame_time = 0.0f;
    int chunk_radius = 8;

    EntityComponentSystem ECS{};
    MovementSystem* movement_system{};
    CameraSystem* camera_system{};
    std::vector<RenderCall> render_queue{};
    EventManager event_manager{};
    std::queue<Event> events{};
    ClientInterface client{};

    // TODO: create shader manager
    GLuint voxel_shader{};

    // TODO: Add layers
    GLuint HUD_VAO{};
    GLuint HUD_VBO{};

    std::unique_ptr<ShadowMap> shadow_map{};

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
};
