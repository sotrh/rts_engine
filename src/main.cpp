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

    engine::InputManager input(window);

    engine::EntityRenderer renderer;
    renderer.init();
    engine::SelectionBoxRenderer selectionRenderer;
    selectionRenderer.init();

    engine::World world(renderer, selectionRenderer);

    double lastTime = glfwGetTime();
    double currentTime;
    double deltaTime;

    bool isRightMouseButtonPressed = false;
    bool isLeftMouseButtonPressed = false;
    bool isSelecting = false;
    float startX, startY, curX, curY;
    engine::Selection selection(0, 0, 0, 0, 0);

    input.registerKeyCallback([&](engine::InputManager* input, int key, int scancode, int action, int mods){
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    input.registerMouseButtonCallback([&](engine::InputManager* input, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            isLeftMouseButtonPressed = action == GLFW_PRESS;
            if (!isLeftMouseButtonPressed) {
                world.deselect(selection);
                isSelecting = false;
            }
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (!isRightMouseButtonPressed && action == GLFW_PRESS) {
                world.addTarget(glm::vec3(curX / 800.0f * 2 - 1, 1 - curY / 600.0f * 2, 0.0f));
            }
            isRightMouseButtonPressed = action == GLFW_PRESS;
        }
    });

    input.registerCursorPosCallback([&](engine::InputManager* input, double xpos, double ypos) {
        curX = (float) xpos;
        curY = (float) ypos;

        if (isLeftMouseButtonPressed && !isSelecting) {
            world.deselect(selection);
            isSelecting = true;
            startX = curX;
            startY = curY;
        } else if (!isLeftMouseButtonPressed) {
            world.deselect(selection);
            isSelecting = false;
        }
        
        if (isSelecting) {
            selection.minX = (startX < curX ? startX : curX) / 800.0f * 2 - 1;
            selection.minY = 1 - (startY > curY ? startY : curY) / 600.0f * 2;
            selection.maxX = (startX > curX ? startX : curX) / 800.0f * 2 - 1;
            selection.maxY = 1 - (startY < curY ? startY : curY) / 600.0f * 2;
            world.select(selection);
        }
    });

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    selectionRenderer.cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}