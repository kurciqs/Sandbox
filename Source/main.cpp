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

    ParticleSystem particleSystem(10, ParticleSystemType::Testing);
    float particleSpawnDebounce = 0.1f;
    bool runSimulation = true;
    float runSimulationDebounce = 0.1f;

    FPSCounter::Init();
    while (!window.ShouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.3f, 0.3f });

        if (runSimulation)
            particleSystem.Update(DELTA_TIME);
        particleSystem.Draw(renderer);

        renderer.DrawLineCube(lowerBoundary, upperBoundary - lowerBoundary, glm::vec3(0.5f, 0.6f, 0.7f));
        renderer.DrawCube(glm::vec3(-0.25f), glm::vec3(0.5f), glm::vec3(1.0f));

        renderer.Update(DELTA_TIME);
        renderer.Render();

        FPSCounter::Tick();
        if (Input::isKeyDown(GLFW_KEY_F5)) FPSCounter::Print();

        window.CheckBasicInput();
        window.PollEvents();
        window.SwapBuffers();

        if (Input::isKeyDown(GLFW_KEY_T) && runSimulationDebounce < 0.0f) {
            runSimulation = !runSimulation;
            runSimulationDebounce = 0.5f;
        }
        if (particleSpawnDebounce < 0.0f) {
            if (Input::isKeyDown(GLFW_KEY_F))
                particleSystem.AddParticle(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, RANDOM_COLOR, 0.5f);
            else if (Input::isKeyDown(GLFW_KEY_G))
                particleSystem.AddBall(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 2.5f, RANDOM_COLOR);
            particleSpawnDebounce = 0.1f;
        }
        particleSpawnDebounce -= DELTA_TIME;
        runSimulationDebounce -= DELTA_TIME;
    }

    window.Destroy();
    renderer.Shutdown();
    particleSystem.Destroy();
    Window::ShutdownGlfw();

    return 0;
}
