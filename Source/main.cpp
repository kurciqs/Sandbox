#include "Window/Window.h"
#include "Simulation/ParticleSystem.h"
#define DELTA_TIME 0.02f

int main() {

    // TODO: figure pwd out
    chdir("../..");

    if (!Window::InitGlfw()) {
        return -1;
    }
    Window window(1200, 800, "Sandbox");
    window.MakeContextCurrent();
    window.SetIcon("Assets/Images/Icon.png");
    window.InstallCallbacks();

    if (!Renderer::InitGlad()) {
        return -1;
    }
    Renderer renderer(&window);

    ParticleSystem particleSystem(7, ParticleSystemType::Testing);

    FPSCounter::Init();
    while (!window.ShouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.3f, 0.3f });

        particleSystem.Update(DELTA_TIME);
        particleSystem.Draw(renderer);

        renderer.DrawCube(glm::vec3(-0.25f), glm::vec3(0.5f), glm::vec3(1.0f));

        renderer.Update(DELTA_TIME);
        renderer.Render();

        FPSCounter::Tick();
        if (Input::isKeyDown(GLFW_KEY_F5)) FPSCounter::Print();
        window.CheckBasicInput();
        window.PollEvents();
        window.SwapBuffers();
    }

    window.Destroy();
    renderer.Shutdown();
    particleSystem.Destroy();
    Window::ShutdownGlfw();

    return 0;
}
