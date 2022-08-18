#include "Window/Window.h"
#include "Simulation/ParticleSystem.h"
#define DELTA_TIME 0.02f

int main() {
    // set the workdir to ../../ from the executable
    wchar_t exepath[MAX_PATH];
    getexepath(exepath);
    auto path = std::filesystem::path(exepath);
    std::filesystem::current_path(path.parent_path().parent_path().parent_path());

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
    particleSystem.AddCube(glm::vec3(8.0f), glm::vec3(0.0f), 5, 5, 5, RANDOM_COLOR);
//    particleSystem.AddBall(glm::vec3(0.0f), glm::vec3(0.0f), 4.0f, RANDOM_COLOR);
    float particleSpawnDebounce = 0.2f;
    bool runSimulation = true;
    float runSimulationDebounce = 0.1f;

    while (!window.ShouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.3f, 0.3f });

        if (runSimulation)
            particleSystem.Update(DELTA_TIME);
        particleSystem.Draw(renderer);

        renderer.DrawLineCube(lowerBoundary, upperBoundary - lowerBoundary, glm::vec3(0.5f, 0.6f, 0.7f));
        renderer.DrawCube(glm::vec3(-0.25f), glm::vec3(0.5f), glm::vec3(1.0f));
        // Draw the ground
        renderer.DrawTriangle(lowerBoundary, glm::vec3(lowerBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(upperBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(0.4f));
        renderer.DrawTriangle(lowerBoundary, glm::vec3(upperBoundary.x, lowerBoundary.y, lowerBoundary.z), glm::vec3(upperBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(0.4f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        renderer.Update(DELTA_TIME);
        renderer.Render();

        window.CheckBasicInput();
        window.PollEvents();
#ifndef NDEBUG
        window.SwapBuffers(true);
#else
        window.SwapBuffers();
#endif

        if (Input::isKeyDown(GLFW_KEY_T) && runSimulationDebounce < 0.0f) {
            runSimulation = !runSimulation;
            runSimulationDebounce = 0.5f;
        }
        if (particleSpawnDebounce < 0.0f) {
            if (Input::isKeyDown(GLFW_KEY_F))
                particleSystem.AddParticle(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, RANDOM_COLOR, 0.5f);
            else if (Input::isKeyDown(GLFW_KEY_Q))
                particleSystem.AddBall(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 4.0f, RANDOM_COLOR);
            else if (Input::isKeyDown(GLFW_KEY_E))
                particleSystem.AddCube(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 4, 4, 4, RANDOM_COLOR);
            particleSpawnDebounce = 0.2f;
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
