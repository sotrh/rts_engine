#ifndef RTS_INPUT_H
#define RTS_INPUT_H

#include <vector>
#include <iostream>
#include <exception>
#include <functional>
#include <GLFW/glfw3.h>

namespace engine {

    class InputManager {
    public:
        InputManager(GLFWwindow* window) : _window(window) {
            if (glfwGetWindowUserPointer(_window)) {
                throw std::runtime_error("Specified window is already in use");
            }
            glfwSetWindowUserPointer(_window, this);

            glfwSetKeyCallback(_window, KEY_CALLBACK);
            glfwSetCursorPosCallback(_window, CURSOR_POS_CALLBACK);
            glfwSetMouseButtonCallback(_window, MOUSE_BUTTON_CALLBACK);
        }

        using KeyCallback = std::function<void(InputManager* input, int key, int scancode, int action, int mods)>;
        using CursorPosCallback = std::function<void(InputManager* input, double xpos, double ypos)>;
        using MouseButtonCallback = std::function<void(InputManager* input, int button, int action, int mods)>;
        using DragCallback = std::function<void(InputManager* input, double startedX, double startedY, double endedX, double endedY)>;

        void registerKeyCallback(KeyCallback callback) { _keyCallbacks.push_back(callback); }
        void registerCursorPosCallback(CursorPosCallback callback) { _cursorPosCallbacks.push_back(callback); }
        void registerMouseButtonCallback(MouseButtonCallback callback) { _mouseButtonCallbacks.push_back(callback); }
        void registerMouseButtonJustPressedCallback(MouseButtonCallback callback) { _mouseButtonJustPressedCallbacks.push_back(callback); }

        void registerDragStartedCallback(DragCallback callback) { _dragStartedCallbacks.push_back(callback); }
        void registerDragMovedCallback(DragCallback callback) { _dragMovedCallbacks.push_back(callback); }
        void registerDragEndedCallback(DragCallback callback) { _dragEndedCallbacks.push_back(callback); }

    private:
        GLFWwindow* _window;

        std::vector<KeyCallback> _keyCallbacks;
        std::vector<CursorPosCallback> _cursorPosCallbacks;

        std::vector<MouseButtonCallback> _mouseButtonCallbacks;
        std::vector<MouseButtonCallback> _mouseButtonJustPressedCallbacks;

        std::vector<DragCallback> _dragStartedCallbacks;
        std::vector<DragCallback> _dragMovedCallbacks;
        std::vector<DragCallback> _dragEndedCallbacks;

        double cursorX, cursorY;
        double dragStartX, dragStartY;
        bool isDragActive = false;

        bool mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST + 1];

        static void KEY_CALLBACK(GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto input = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
            for (auto c : input->_keyCallbacks) c(input, key, scancode, action, mods);
        }

        static void CURSOR_POS_CALLBACK(GLFWwindow* window, double xpos, double ypos) {
            auto input = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
            for (auto c : input->_cursorPosCallbacks) c(input, xpos, ypos);

            input->cursorX = xpos;
            input->cursorY = ypos;

            if (input->isDragActive) {
                for (auto c : input->_dragMovedCallbacks) c(input, input->dragStartX, input->dragStartY, xpos, ypos);
            }
        }

        static void MOUSE_BUTTON_CALLBACK(GLFWwindow* window, int button, int action, int mods) {
            auto input = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
            if (!input->mouseButtonsPressed[button] && action == GLFW_PRESS) {
                for (auto c : input->_mouseButtonJustPressedCallbacks) c(input, button, action, mods);
            }

            bool isActionPressed = action == GLFW_PRESS;
            input->mouseButtonsPressed[button] = isActionPressed;

            for (auto c : input->_mouseButtonCallbacks) c(input, button, action, mods);

            bool isLeftMouseButtonPressed = isActionPressed && button == GLFW_MOUSE_BUTTON_LEFT;
            if (isLeftMouseButtonPressed) {
                input->dragStartX = input->cursorX;
                input->dragStartY = input->cursorY;
                input->isDragActive = true;
                for (auto c : input->_dragStartedCallbacks) c(input, input->dragStartX, input->dragStartY, input->cursorX, input->cursorY);
            } else {
                for (auto c : input->_dragEndedCallbacks) c(input, input->dragStartX, input->dragStartY, input->cursorX, input->cursorY);
                input->isDragActive = false;
            }
        }
    };
}

#endif//RTS_INPUT_H