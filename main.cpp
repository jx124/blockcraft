#include <GLFW/glfw3.h>
#include "test.hpp"
#include "foo/foo.hpp"

int main() {
    if (!glfwInit()) {
        return 1;
    }

    test_hello();
    test_foo();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello Window", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glClearColor(0.4f, 0.6f, 0.3f, 0.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
