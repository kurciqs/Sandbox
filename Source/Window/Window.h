#ifndef SANDBOX_WINDOW_H
#define SANDBOX_WINDOW_H

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Utils/Utils.h"
#include "Window/Input.h"
#include "glm/glm.hpp"

static int maxWidth = 0;
static int maxHeight = 0;
static int maxRefreshRate = 0;

class Window {
public:
    // Management
    Window(int width_, int height_, const std::string& title_);
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    ~Window();
    void destroy();
    void enterFullscreen();
    void exitFullscreen();
    void checkBasicInput();
    void installCallbacks();
    void setIcon(const std::string& path);

    // Functions
    void hideCursor();
    void showCursor();
    void setCursorPos(const glm::ivec2 &pos);
    glm::ivec2 getCursorPos();
    float getAspectRatio();

    // GLFW related functions
    GLFWwindow* getNativeWindow() const;
    void makeContextCurrent();
    void pollEvents();
    void swapBuffers();
    void close();
    bool shouldClose();
    friend void resizeCallback(GLFWwindow* window, int width, int height);
    friend class Camera;
    friend class Renderer;

private:
    int m_height;
    int m_width;
    // Members
    GLFWwindow* m_window;
    std::string m_title;
    bool m_fullscreen;
    int m_posX;
    int m_posY;
    int m_lastWidth;
    int m_lastHeight;
    float m_fullscreenKeyDebounce = 5.0f;
};


#endif //SANDBOX_WINDOW_H
