#ifndef SANDBOX_INPUT_H
#define SANDBOX_INPUT_H

#include "GLFW/glfw3.h"

namespace Input {
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mouseCallback(GLFWwindow *window, double xpos, double ypos);

    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    void mouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);

    bool isKeyDown(int key);

    bool isMouseButtonDown(int mouseButton);

    void getCursorPos(float *x, float *y);

    bool isMouseButtonUp(int mouseButton);
}

#endif //SANDBOX_INPUT_H
