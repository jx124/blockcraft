#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"

#include "graphics/common.hpp"
#include "graphics/shader.hpp"
#include "graphics/window.hpp"

int main() {
    Window window(800, 600, "Hello Window");

    glClearColor(0.4f, 0.6f, 0.3f, 0.0f);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float vertices[] = {
        -0.6f, -0.4f,
         0.6f, -0.4f,
          0.f,  0.6f,
    };

    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertices, GL_STATIC_DRAW);

    std::optional<GLuint> program = Shader::create("data/shaders/plain.vert", "data/shaders/plain.frag");
    if (!program) {
        return 1;
    }

    const GLint mvp_location = glGetUniformLocation(*program, "MVP");

    while (!glfwWindowShouldClose(window.ptr())) {
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.f, 100.0f);
        
        glUseProgram(*program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window.ptr());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
