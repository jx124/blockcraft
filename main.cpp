#include <iostream>

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#include "graphics/common.hpp"
#include "graphics/window.hpp"

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec2 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vec3(1.0, 0.5, 0.1);\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

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

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    int success;
    char log[512];
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        std::cerr << "Error compiling vertex shader:\n\t" << log << std::endl;
    }

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    
    if (!success) {
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        std::cerr << "Error compiling fragment shader:\n\t" << log << std::endl;
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cerr << "Error linking shader: " << log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    const GLint mvp_location = glGetUniformLocation(program, "MVP");

    while (!glfwWindowShouldClose(window.ptr())) {
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4 mvp = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.f, 100.0f);
        
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window.ptr());
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
