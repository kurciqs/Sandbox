#include "Window/Window.h"
#include "Graphics/Renderer.h"

#include <vector>

int main() {
    if (!glfwInit()) {
        print_error("Failed to initialize GLFW", 0);
        return -1;
    }

    // TODO: figure pwd out
    chdir("../..");

    Window window(1200, 800, "Sandbox");
    window.makeContextCurrent();
    window.setIcon("Resources/Images/Icon.png");

    if (!Renderer::InitGL()) {
        print_error("Failed to initialize GL!", 0);
        return -1;
    }
    window.installCallbacks();

    Renderer renderer(&window);

    float rot = 0.0f;
    glm::vec4 color(0.2f, 0.3f, 0.3f, 1.0f);
    glm::vec3 pos(0.0f);
    while (!window.shouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor(color);

        renderer.DrawDemo();

        renderer.Update(0.02f);
        renderer.Render();

        window.checkBasicInput();
        window.pollEvents();
        window.swapBuffers();
    }

    renderer.Shutdown();
    window.destroy();
    glfwTerminate();

    return 0;
}
