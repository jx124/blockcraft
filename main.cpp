#include "glm/ext/matrix_transform.hpp"
#include <GL/gl.h>
#include <GL/glext.h>
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include "graphics/common.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "graphics/window.hpp"

int main() {
    Window window(800, 600, "Hello Window");

    glClearColor(0.4f, 0.6f, 0.3f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    float vertices[] = {
        // coords      // uv-coords
        -1.0f, -1.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  0.0f,  1.0f,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    std::optional<GLuint> program = Shader::create("data/shaders/plain.vert", "data/shaders/plain.frag");
    if (!program) {
        return 1;
    }

    std::optional<ImageData> stone = Texture::read_image("data/assets/glass.png");
    if (!stone) {
        return 1;
    }

    Texture stone_texture = Texture(*stone, GL_TEXTURE_2D);

    const GLint mvp_location = glGetUniformLocation(*program, "MVP");
    const GLint texture_location = glGetUniformLocation(*program, "textureId");

    while (!glfwWindowShouldClose(window.ptr())) {
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 model(1.0f);
        model = glm::rotate(model, (float)glfwGetTime(), {0.0f, 0.0f, 1.0f});
        model = glm::scale(model, {0.5f, 0.5f, 1.0f});

        constexpr float aspect = 800.0f / 600.0f;
        glm::mat4 proj = glm::ortho(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f, -100.f, 100.0f);
        glm::mat4 mvp = proj * model;
        
        glUseProgram(*program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(texture_location, stone_texture.get_slot());

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window.ptr());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
