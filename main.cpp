#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include "graphics/common.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/window.hpp"

#include "utils/logger.hpp"

struct RenderCall {
    glm::mat4 transform;
    GLuint VAO;
    GLuint shader_id;
    GLuint texture_unit;
    size_t n_vertices;
};

void render(RenderCall render_call) {
    glUseProgram(render_call.shader_id);
    Shader::set_uniform(render_call.shader_id, "textureId", (int)render_call.texture_unit);
    Shader::set_uniform(render_call.shader_id, "model", render_call.transform);

    glBindVertexArray(render_call.VAO);
    glDrawArrays(GL_TRIANGLES, 0, render_call.n_vertices);
}

int main() {
    Window window(800, 600, "Hello Window");

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
        return 1;
    }

    std::optional<ImageData> glass = Texture::read_image("data/assets/glass.png");
    if (!glass) {
        return 1;
    }

    Texture glass_texture = Texture(*glass, GL_TEXTURE_2D);

    std::optional<ImageData> stone = Texture::read_image("data/assets/stone.png");
    if (!stone) {
        return 1;
    }

    Texture stone_texture = Texture(*stone, GL_TEXTURE_2D);

    log_debug_line("stone texture location: %p", &stone_texture);

    while (!glfwWindowShouldClose(window.ptr())) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

        glUseProgram(*program);

        Camera& camera = window.camera;
        if (window.state.go_forward) {
            camera.camera_pos += camera.camera_speed * camera.camera_front;
        }
        if (window.state.go_backward) {
            camera.camera_pos -= camera.camera_speed * camera.camera_front;
        }
        if (window.state.go_left) {
            camera.camera_pos -= glm::normalize(glm::cross(camera.camera_front, camera.camera_up)) * camera.camera_speed;
        }
        if (window.state.go_right) {
            camera.camera_pos += glm::normalize(glm::cross(camera.camera_front, camera.camera_up)) * camera.camera_speed;
        }

        glm::mat4 view = glm::lookAt(camera.camera_pos, camera.camera_pos + camera.camera_front, camera.camera_up);
        Shader::set_uniform(*program, "view", view);

        constexpr float aspect = 800.0f / 600.0f;
        glm::mat4 proj = glm::perspective(glm::radians(camera.fov), aspect, 0.1f, 1000.0f);
        Shader::set_uniform(*program, "proj", proj);

        render(call_1);
        render(call_2);
        
        glfwSwapBuffers(window.ptr());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
