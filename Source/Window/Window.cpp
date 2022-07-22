#include "Window.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static void errorCallback(int error, const char* description) {
    print_error("GLFW Error %d: %s", error, description);
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    auto* win = (Window*)glfwGetWindowUserPointer(window);

    if (width != 0 && height != 0) {
        win->m_width = width;
        win->m_height = height;
        glViewport(0, 0, width, height);
    }
}

Window::Window(int width, int height, const std::string& title)
: m_width(width), m_height(height), m_title(title), m_fullscreen(false)
{
    if (!glfwInitialized) {
        print_error("GLFW not initialized, failed to create window.\n", 0);
        return;
    }
    if (!maxWidth && !maxHeight) {
        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        maxWidth = mode->width;
        maxHeight = mode->height;
        maxRefreshRate = mode->refreshRate;
    }

    if (width < 0 || height < 0) {
        print_error("Invalid window size, creating window with maximum size.", 0);
        m_width = maxWidth;
        m_height = maxHeight;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        print_error("Failed to create GLFW window", 0);
        glfwTerminate();
    }

    glfwSetWindowUserPointer(m_window, (void*)this);


    glfwSwapInterval(1);
}

void Window::MakeContextCurrent() {
    if (m_window) {
        glfwMakeContextCurrent(m_window);
    }
}

void Window::PollEvents() {
    if (m_window) {
        glfwPollEvents();
    }
}

void Window::SwapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void Window::Close() {
    if (m_window) {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

bool Window::ShouldClose() {
    if (m_window) {
        return glfwWindowShouldClose(m_window);
    }
    return true;
}

void Window::InstallCallbacks() {
    if (m_window) {
        glfwSetErrorCallback(errorCallback);
        glfwSetWindowSizeCallback(m_window, resizeCallback);
        glfwSetKeyCallback(m_window, Input::keyCallback);
        glfwSetCursorPosCallback(m_window, Input::mouseCallback);
        glfwSetMouseButtonCallback(m_window, Input::mouseButtonCallback);
        glfwSetScrollCallback(m_window, Input::mouseScrollCallback);
    }
}

void Window::CheckBasicInput() {
    if (m_window) {
        m_fullscreenKeyDebounce -= 0.02f;
        if (Input::isKeyDown(GLFW_KEY_ESCAPE)) {
            Close();
        }
        if (Input::isKeyDown(GLFW_KEY_F11) && m_fullscreenKeyDebounce < 0.0f) {
            if (m_fullscreen)
                ExitFullscreen();
            else
                EnterFullscreen();
            m_fullscreenKeyDebounce = 0.5f;
        }
    }
}

void Window::ShowCursor() {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::HideCursor() {
    if (m_window) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
}

glm::ivec2 Window::GetCursorPos() {
    if (m_window) {
        double x, y;
        glfwGetCursorPos(m_window, &x, &y);
        return {x, y};
    }
    return {0, 0};
}

void Window::SetCursorPos(const glm::ivec2 &pos) {
    if (m_window) {
        glfwSetCursorPos(m_window, pos.x, pos.y);
    }
}

void Window::Destroy() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Window::EnterFullscreen() {
    if (m_window) {
        m_fullscreen = true;
        m_lastWidth = m_width;
        m_lastHeight = m_height;
        glfwGetWindowPos(m_window, &m_posX, &m_posY);
        glfwSetWindowMonitor(m_window, glfwGetPrimaryMonitor(), maxWidth / 2, maxHeight / 2, maxWidth, maxHeight, maxRefreshRate);
        m_width = maxWidth;
        m_height = maxHeight;
        glfwSwapInterval(0);
        glfwSwapInterval(1);
    }
}

void Window::ExitFullscreen() {
    if (m_window) {
        m_fullscreen = false;
        glfwSetWindowMonitor(m_window, nullptr, m_posX, m_posY, m_lastWidth, m_lastHeight, maxRefreshRate);
        m_width = m_lastWidth;
        m_height = m_lastHeight;
        glfwSwapInterval(0);
        glfwSwapInterval(1);
    }
}

void Window::SetIcon(const std::string& path) {
    if (m_window) {
        GLFWimage image;
        image.pixels = stbi_load(path.c_str(), &image.width, &image.height, nullptr, STBI_rgb_alpha);
        if (!image.pixels) {
            print_error("Failed to load icon %s. Reason: %s\n", path.c_str(), stbi_failure_reason());
        }
        glfwSetWindowIcon(m_window, 1, &image);
        stbi_image_free(image.pixels);
    }
}

GLFWwindow *Window::GetNativeWindow() const {
    return m_window;
}

float Window::GetAspectRatio() {
    return (float)(m_width) / (float)(m_height);
}

bool Window::InitGlfw() {
    if (!glfwInit()) {
        print_error("Failed to initialize GLFW.\n", 0);
        return false;
    }
    glfwInitialized = true;
    return true;
}

void Window::ShutdownGlfw() {
    if (glfwInitialized) {
        glfwTerminate();
        glfwInitialized = false;
    }
}

void Window::SetTitle(const std::string& title) {
    m_title = title;
    if (m_window) {
        glfwSetWindowTitle(m_window, title.c_str());
    }
}
