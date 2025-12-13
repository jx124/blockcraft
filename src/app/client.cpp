#include "blocks/chunk_manager.hpp"
#include "texture_manager.hpp"
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include "app/client.hpp"

#include "blocks/chunk.hpp"
#include "graphics/shader.hpp"
#include "utils/logger.hpp"

ClientApplication::ClientApplication(int width, int height) : width(width), height(height), window(nullptr) {
    if (!glfwInit()) {
        throw std::runtime_error("[ClientApplication] Cannot initialize GLFW");
    }

    window = std::make_unique<Window>(width, height, "Blockcraft Client");
    window->create();

    // save a pointer to this ClientApplication instance; to be accessed in callbacks
    glfwSetWindowUserPointer(window->ptr(), this);

    glfwSetFramebufferSizeCallback(window->ptr(), framebuffer_size_callback);
    glfwSetInputMode(window->ptr(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window->ptr(), key_callback);
    glfwSetCursorPosCallback(window->ptr(), cursor_pos_callback);
    glfwSetMouseButtonCallback(window->ptr(), mouse_button_callback);
    glfwSetScrollCallback(window->ptr(), scroll_callback);

#ifdef DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(error_message_callback, nullptr);
    log_debug("OpenGL Version %s", glGetString(GL_VERSION));
#endif
}

ClientApplication::~ClientApplication() {
    window->destroy();
    glfwTerminate();
}

void ClientApplication::run() {
    glClearColor(0.4f, 0.75f, 0.9f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    GLuint voxel_shader = Shader::create("data/shaders/voxel.vert", "data/shaders/voxel.frag").value_or(0);
    if (!voxel_shader) {
        return;
    }

    TextureManager texture_manager;

    int seed = 2345;
    int chunk_radius = 10;
    ChunkManager chunk_manager(seed, chunk_radius);

    physics_system = ECS.register_system<PhysicsSystem>();
    movement_system = ECS.register_system<MovementSystem>();
    camera_system = ECS.register_system<CameraSystem>();

    ECS.add_components_to_system<PhysicsSystem, Transform, Velocity>();
    ECS.add_components_to_system<MovementSystem, Transform, Velocity, PlayerMovement>();

    EntityID player = ECS.create_entity().value();
    ECS.add_component_to_entity(player, Transform{{8.0f, 8.0f, 160.0f}, {}});
    ECS.add_component_to_entity(player, Velocity{});
    ECS.add_component_to_entity(player, PlayerMovement{});
    ECS.add_component_to_entity(player, Camera{});

    ECS.clear_unused_archetypes();

    event_manager.subscribe_system_to_event<InputEvent>(physics_system);
    event_manager.subscribe_system_to_event<MovementEvent>(movement_system);

    camera_system->register_primary_camera();

    while (!should_close) {
        glfwPollEvents();

        event_manager.process_events(this->events);
        this->update();

        movement_system->update(dt, chunk_manager);
        physics_system->update(dt);

        chunk_manager.load_chunks(1, texture_manager);
        chunk_manager.unload_chunks(1);

        glUseProgram(voxel_shader);
        glm::mat4 identity(1.0f);

        for (const auto& chunk : chunk_manager.get_chunks()) {
            render_queue.emplace_back(
                glm::translate(identity, chunk.to_world_pos(glm::vec3(0.0f))),
                chunk.get_VAO(),
                voxel_shader,
                texture_manager.get_texture_unit(),
                chunk.get_num_vertices()
            );
        }

        Shader::set_uniform(voxel_shader, "view", camera_system->view());
        Shader::set_uniform(voxel_shader, "projection", camera_system->projection());

        this->render();
        window->update();

        if (window->should_close()) {
            this->stop();
        }
    }
}

// Update application specific events and fields
void ClientApplication::update() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dt = (float)glfwGetTime() - previous_time;
    previous_time = (float)glfwGetTime();

    while (!events.empty()) {
        Event event = std::move(events.front());
        EventType event_type = event.event;
        events.pop();

        ApplicationEvent* window_event = std::get_if<ApplicationEvent>(&event_type);
        switch (window_event->type) {
            case ApplicationEvent::Type::CloseWindow:
                stop();
                break;
            case ApplicationEvent::Type::ToggleCursor:
                glfwSetInputMode(window->ptr(), GLFW_CURSOR, cursor_disabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                cursor_disabled = !cursor_disabled;
                if (!cursor_disabled) {
                    movement_system->reset_first_mouse();
                }
                break;
        }
    }
}

// TODO: add layers
void ClientApplication::render() {
    for (const RenderCall& render_call : render_queue) {
        glUseProgram(render_call.shader_id);
        Shader::set_uniform(render_call.shader_id, "textureId", (int)render_call.texture_unit);
        Shader::set_uniform(render_call.shader_id, "model", render_call.transform);

        glBindVertexArray(render_call.VAO);
        glDrawArrays(GL_TRIANGLES, 0, render_call.n_vertices);
    }
    render_queue.clear();
}

// TODO: save data to disk
void ClientApplication::stop() {
    should_close = true;
}

void ClientApplication::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    ClientApplication* app_ptr = static_cast<ClientApplication*>(glfwGetWindowUserPointer(window));

    app_ptr->width = width;
    app_ptr->height= height;
    app_ptr->window->width = width;
    app_ptr->window->height= height;

    glViewport(0, 0, width, height);
}

void ClientApplication::error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
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

void ClientApplication::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode, (void)mods;

    ClientApplication* app_ptr = static_cast<ClientApplication*>(glfwGetWindowUserPointer(window));

    app_ptr->event_manager.queue_input_event(
            Event::make_event(action == GLFW_PRESS 
                ? InputEvent::Type::KeyPress 
                : action == GLFW_RELEASE
                ? InputEvent::Type::KeyRelease 
                : InputEvent::Type::KeyRepeat, key));
}

void ClientApplication::cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
    ClientApplication* app_ptr = static_cast<ClientApplication*>(glfwGetWindowUserPointer(window));

    app_ptr->event_manager.queue_input_event(
            Event::make_event(InputEvent::Type::MouseMove, 0, static_cast<float>(x_pos), static_cast<float>(y_pos)));
}

void ClientApplication::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window, (void)button, (void)action, (void)mods;
    log_debug("button: %d, action: %s", button,
        action == GLFW_PRESS 
            ? "press" 
            : action == GLFW_RELEASE
            ? "release"
            : "repeat");
}

void ClientApplication::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window, (void)xoffset, (void)yoffset;
}
