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

    if (!Renderer::InitGlad()) {
        return -1;
    }

    window.InstallCallbacks();

    Renderer renderer(&window);

    FPSCounter::Init();
    while (!window.ShouldClose()) {
        FPSCounter::Tick();

        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.2f, 0.2f });

        renderer.DrawCube(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(1.0f));

        renderer.Update(0.02f);
        renderer.Render();

        if (Input::isKeyDown(GLFW_KEY_F5)) FPSCounter::Print();
        window.CheckBasicInput();
        window.PollEvents();
        window.SwapBuffers();
    }

    renderer.Shutdown();
    window.Destroy();
    Window::ShutdownGlfw();

    return 0;
}
