#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine.h"

int main(int argc, char** argv) {
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error (" << error << "):\n" << description << std::endl;
    });

    if (!glfwInit()) {
        std::cerr << "Unable to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(800, 600, "RTS Engine", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Unable to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Unable to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    engine::DebugRenderer renderer;
    renderer.init();
    engine::World world(renderer);

    double lastTime = glfwGetTime();
    double currentTime;
    double deltaTime;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        currentTime = glfwGetTime();
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        world.update(deltaTime);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    renderer.cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}