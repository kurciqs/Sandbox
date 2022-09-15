#include "Window/Window.h"
#include "Simulation/ParticleSystem.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
//    IM_UNUSED(io);
    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window.GetNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 460");

    io.Fonts->AddFontDefault();
    ImFont* mainFont = io.Fonts->AddFontFromFileTTF(R"(Assets/Fonts/RandyGG.ttf)", 17.0f);
    IM_ASSERT(mainFont != nullptr);

    ParticleSystem particleSystem(500, ParticleSystemType::Pool);

    float particleSpawnDebounce = 0.2f;
    bool runSimulation = true;
    float runSimulationDebounce = 0.1f;
    bool menu = false;
    float menuSelectDebounce = 0.5f;

    const char* spawnObjectLabels[5] = {"Cube", "Cone", "Cylinder", "Torus", "Ball"};
    int spawnObjectSelected = 0;
    const char* spawnObjectPreview = " ";
    char spawnObjectPath[MAX_PATH] = " ";

    glm::vec3 gravity(0.0f, -9.8f, 0.0f);
    while (!window.ShouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor({ 0.2f, 0.3f, 0.3f });

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        particleSystem.SetGlobalForce(gravity);

        if (runSimulation)
            particleSystem.Update(DELTA_TIME);
        particleSystem.Draw(renderer);

        renderer.DrawLineCube(lowerBoundary, upperBoundary - lowerBoundary, glm::vec3(0.5f, 0.6f, 0.7f));
        // Draw the ground
        renderer.DrawTriangle(lowerBoundary, glm::vec3(lowerBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(upperBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(0.4f));
        renderer.DrawTriangle(lowerBoundary, glm::vec3(upperBoundary.x, lowerBoundary.y, lowerBoundary.z), glm::vec3(upperBoundary.x, lowerBoundary.y, upperBoundary.z), glm::vec3(0.4f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        if (!menu) {
            renderer.Update(DELTA_TIME);
        }
        renderer.Render();

        if (menu) {
            ImGui::PushFont(mainFont);

            ImGui::Begin("Simulation parameters");

            // Text that appears in the window
            ImGui::Text("Press TAB to exit the menu.");
            ImGui::SliderFloat3("Gravity", &gravity[0], -10.0f, 10.0f);

            ImGui::Combo("Select Object to spawn", &spawnObjectSelected, spawnObjectLabels, IM_ARRAYSIZE(spawnObjectLabels));

            ImGui::InputText("Load Model", spawnObjectPath, IM_ARRAYSIZE(spawnObjectPath));

            ImGui::End();
            ImGui::PopFont();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

#ifndef NDEBUG
        window.SwapBuffers(true);
#else
        window.SwapBuffers();
#endif
        window.PollEvents();
        window.CheckBasicInput();

        if (!menu) {
            if (Input::isKeyDown(GLFW_KEY_T) && runSimulationDebounce < 0.0f) {
                runSimulation = !runSimulation;
                runSimulationDebounce = 0.5f;
            }
            if (particleSpawnDebounce < 0.0f) {
                if (Input::isKeyDown(GLFW_KEY_F))
                    particleSystem.AddParticle(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, RANDOM_COLOR, 0.5f);
                else if (Input::isKeyDown(GLFW_KEY_E)) {
                    switch (spawnObjectSelected) {
                        case 0:
                            particleSystem.AddCube(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 3, 3, 3, RANDOM_COLOR);
                            break;
                        case 1:
                            particleSystem.AddCone(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, glm::radians(45.0f), 4.0f,
                                                   RANDOM_COLOR);
                            break;
                        case 2:
                            particleSystem.AddCylinder(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 5.0f, 2.5f, RANDOM_COLOR);
                            break;
                        case 3:
                            particleSystem.AddTorus(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 1.0f, 4.5f, RANDOM_COLOR);
                            break;
                        case 4:
                            particleSystem.AddBall(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 3.0f, RANDOM_COLOR);
                            break;
                        default:
                            break;
                    }
                }
                particleSpawnDebounce = 0.2f;
            }
        }


        if (menuSelectDebounce < 0.0f && Input::isKeyDown(GLFW_KEY_TAB)) {
            menu = !menu;
            menuSelectDebounce = 0.5f;
        }


        particleSpawnDebounce -= DELTA_TIME;
        runSimulationDebounce -= DELTA_TIME;
        menuSelectDebounce -= DELTA_TIME;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    window.Destroy();
    renderer.Shutdown();
    particleSystem.Destroy();
    Window::ShutdownGlfw();

    return 0;
}
