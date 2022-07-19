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
static bool glfwInitialized = false;

class Window {
public:
    // Management
    static bool InitGlfw();
    static void ShutdownGlfw();
    Window(int width_, int height_, const std::string& title_);
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    ~Window() = default;
    void Destroy();
    void EnterFullscreen();
    void ExitFullscreen();
    void CheckBasicInput();
    void InstallCallbacks();

    // Functions
    void HideCursor();
    void SetIcon(const std::string& path);\
    void SetTitle(const std::string& title);
    void ShowCursor();
    void SetCursorPos(const glm::ivec2 &pos);
    glm::ivec2 GetCursorPos();
    float GetAspectRatio();

    // GLFW related functions
    [[nodiscard]] GLFWwindow* GetNativeWindow() const;
    void MakeContextCurrent();
    void PollEvents();
    void SwapBuffers();
    void Close();
    bool ShouldClose();
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
    int m_posX = 0;
    int m_posY = 0;
    int m_lastWidth = 0;
    int m_lastHeight = 0;
    float m_fullscreenKeyDebounce = 5.0f;
};


#endif //SANDBOX_WINDOW_H
