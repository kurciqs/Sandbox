#include "Window/Window.h"
#include "Graphics/Renderer.h"

int main() {

    // TODO: figure pwd out
    chdir("../..");

    if (!Window::InitGlfw()) {
        return -1;
    }

    Window window(1200, 800, "Sandbox");
    window.MakeContextCurrent();
    window.SetIcon("Resources/Images/Icon.png");

    if (!Renderer::InitGL()) {
        print_error("Failed to initialize GL!", 0);
        return -1;
    }
    window.InstallCallbacks();

    Renderer renderer(&window);

    while (!window.ShouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.2f, 0.2f });

        renderer.DrawDemo();

        renderer.Update(0.02f);
        renderer.Render();

        window.CheckBasicInput();
        window.PollEvents();
        window.SwapBuffers();
    }

    renderer.Shutdown();
    window.Destroy();
    glfwTerminate();

    return 0;
}
