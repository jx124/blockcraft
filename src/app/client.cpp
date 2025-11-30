#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include "app/client.hpp"

#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "systems/physics.hpp"

ClientApplication::ClientApplication(int width, int height) : width(width), height(height), window(nullptr) {
    if (!glfwInit()) {
        throw std::runtime_error("[ClientApplication] Cannot initialize GLFW");
    }

    window = std::make_unique<Window>(width, height, "Blockcraft Client");
    window->create();

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

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float vertices[] = {
        // coords         // uv-coords
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

        // top
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        // left
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        // right
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,

        //front
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // back
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    std::optional<GLuint> program = Shader::create("data/shaders/plain.vert", "data/shaders/plain.frag");
    if (!program) {
        return;
    }

    std::optional<ImageData> glass = Texture::read_image("data/assets/glass.png");
    if (!glass) {
        return;
    }

    Texture glass_texture = Texture(*glass, GL_TEXTURE_2D);

    std::optional<ImageData> stone = Texture::read_image("data/assets/stone.png");
    if (!stone) {
        return;
    }

    Texture stone_texture = Texture(*stone, GL_TEXTURE_2D);

    physics_system = ECS.register_system<PhysicsSystem>();

    ECS.add_components_to_system<PhysicsSystem, Transform, Velocity>();

    std::vector<EntityID> entities(10);
    for (auto& e : entities) {
        e = ECS.create_entity().value_or(0);
        ECS.add_component_to_entity(e, Transform{ (float)e * 0.1234f , (float)e * 0.6543f });
        ECS.add_component_to_entity(e, Velocity{ (float)e * 0.7823f , (float)e * 0.6613f });
    }
    ECS.clear_unused_archetypes();

    while (!should_close) {
        glfwPollEvents();

        this->update((float)glfwGetTime());

        glUseProgram(*program);
        glm::mat4 model(1.0f);
        model = glm::translate(model, {0.0f, 2.0f, 0.0f});

        RenderCall call_1 = {
            model,
            VAO,
            *program,
            stone_texture.get_unit(),
            36
        };

        model = glm::translate(model, {-1.0f, 0.0f, 0.0f});

        RenderCall call_2 = {
            model,
            VAO,
            *program,
            glass_texture.get_unit(),
            36
        };

        Camera& camera = window->camera;
        if (window->state.go_forward) {
            camera.camera_pos += camera.camera_speed * camera.camera_front;
        }
        if (window->state.go_backward) {
            camera.camera_pos -= camera.camera_speed * camera.camera_front;
        }
        if (window->state.go_left) {
            camera.camera_pos -= glm::normalize(glm::cross(camera.camera_front, camera.camera_up)) * camera.camera_speed;
        }
        if (window->state.go_right) {
            camera.camera_pos += glm::normalize(glm::cross(camera.camera_front, camera.camera_up)) * camera.camera_speed;
        }

        glm::mat4 view = glm::lookAt(camera.camera_pos, camera.camera_pos + camera.camera_front, camera.camera_up);
        Shader::set_uniform(*program, "view", view);

        constexpr float aspect = 800.0f / 600.0f;
        glm::mat4 proj = glm::perspective(glm::radians(camera.fov), aspect, 0.1f, 1000.0f);
        Shader::set_uniform(*program, "proj", proj);

        render_queue.push_back(std::move(call_1));
        render_queue.push_back(std::move(call_2));

        this->render();
        window->update();

        if (window->should_close()) {
            this->stop();
        }
    }
}

void ClientApplication::update(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //physics_system->print_info();
    physics_system->update(dt);
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
    Window* win_ptr = static_cast<Window*>(glfwGetWindowUserPointer(window));
    win_ptr->width = width;
    win_ptr->height= height;

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

void ClientApplication::cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
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

void ClientApplication::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)window, (void)button, (void)action, (void)mods;
}

void ClientApplication::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)window, (void)xoffset, (void)yoffset;
}
