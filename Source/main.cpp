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
    window.SetIcon("Assets/Images/Icon.jpg");
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

    ParticleSystem particleSystem(400, ParticleSystemType::Pool);

    float particleSpawnDebounce = 0.2f;
    float fluidSpawnDebounce = 0.2f;
    bool runSimulation = false;
    float runSimulationDebounce = 0.1f;
    bool menu = true;
    float menuSelectDebounce = 0.5f;

    const char* spawnObjectLabels[5] = {"Cube", "Cone", "Cylinder", "Torus", "Ball"};
    int spawnObjectSelected = 0;
    char spawnObjectPath[MAX_PATH] = "";
    float spawnObjectMass = 1.0f;
    bool spawnObjectFixed = false;
    glm::vec3 spawnObjectColor = RANDOM_COLOR;

    float spawnFluidViscosity = 0.01f;
    int spawnFluidParticleSize = 100;
    glm::vec3 spawnFluidOffset(0.0f);
    glm::vec3 spawnFluidColor = RANDOM_COLOR;

    int emitFluidID = -1;

    glm::vec3 gravity(0.0f, -10.0f, 0.0f);

    renderer.Update(DELTA_TIME);
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
//        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
//        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//        renderer.DrawLine(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        if (!menu) {
            renderer.Update(DELTA_TIME);
        }
        renderer.Render();

        if (menu) {
            ImGui::PushFont(mainFont);

            ImGui::Begin("Simulation parameters", nullptr, ImGuiWindowFlags_NoTitleBar);

//            if (ImGui::CollapsingHeader("Options")) {
                ImGui::SliderFloat3("Gravity", &gravity[0], -10.0f, 10.0f);
                if (ImGui::CollapsingHeader("Spawn object")) {
                    ImGui::Combo("Select object", &spawnObjectSelected, spawnObjectLabels, IM_ARRAYSIZE(spawnObjectLabels));
                    ImGui::SliderFloat("Object mass", &spawnObjectMass, 0.1f, 25.0f);
                    ImGui::ColorEdit3("Object color", &spawnObjectColor[0]);
                    ImGui::Checkbox("Object fixed", &spawnObjectFixed);
//                    ImGui::InputText("", spawnObjectPath, IM_ARRAYSIZE(spawnObjectPath)); TODO fix imgui crash
                    ImGui::SameLine();
                    if (ImGui::Button("Load"))
                        particleSystem.AddObject(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, spawnObjectPath, spawnObjectMass, spawnObjectFixed);
                }
                if (ImGui::CollapsingHeader("Emit fluid")) {
                    ImGui::SliderInt("Choose fluid to emit", &emitFluidID, -1, particleSystem.GetFluidAmount() - 1 );
                }
                if (ImGui::CollapsingHeader("Spawn fluid")) {
                    ImGui::ColorEdit3("Fluid color", &spawnFluidColor[0]);
                    ImGui::SliderFloat3("Fluid offset", &spawnFluidOffset[0], glm::compMin(lowerBoundary), glm::compMin(upperBoundary));
                    ImGui::SliderFloat("Fluid viscosity", &spawnFluidViscosity, 0.0f, 1.25f);
                    ImGui::SliderInt("Fluid particle amount", &spawnFluidParticleSize, 0, 500);

                    if (ImGui::Button("Spawn"))
                        particleSystem.AddFluid(spawnFluidParticleSize, 5.0f, spawnFluidOffset, spawnFluidColor, 0.2f, spawnFluidViscosity);
                }
//            }

            if (ImGui::CollapsingHeader("Controls")) {
                ImGui::Text("Press WASD to move, Space/LCtrl to ascend/descend.");
                ImGui::Text("Press E to spawn object.");
                ImGui::Text("Press F to spawn single particle.");
                ImGui::Text("Press Q to emit fluid.");
                ImGui::Text("Press T to stop the simulation.");
                ImGui::Text("Press TAB to exit the menu.");
                ImGui::Text("Press ESC to exit the application");
                ImGui::Text("Input path to an obj file into the text box and\npress the Load button to load it into the app.");
            }

            ImGui::End();
//            ImGui::ShowDemoWindow();
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
            if (Input::isKeyDown(GLFW_KEY_Q) && fluidSpawnDebounce < 0.0f) {
                for (float i = -1.0f; i <= 1.0f; i += 1.0f) {
                    for (float j = -1.0f; j <= 1.0f; j += 1.0f) {
                        particleSystem.EmitFluidParticle(emitFluidID, renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f + glm::vec3(i, j, 0.0f), renderer.GetCameraOrientation() * 20.0f);
                    }
                }
                fluidSpawnDebounce = 0.05f;
            }
            if (particleSpawnDebounce < 0.0f) {
                if (Input::isKeyDown(GLFW_KEY_F))
                    particleSystem.AddParticle(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, spawnObjectColor, 0.5f);
                else if (Input::isKeyDown(GLFW_KEY_E)) {
                    switch (spawnObjectSelected) {
                        case 0:
                            particleSystem.AddCube(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 3, 3, 3, spawnObjectColor, spawnObjectMass, spawnObjectFixed);
                            break;
                        case 1:
                            particleSystem.AddCone(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, glm::radians(30.0f), 6.0f,spawnObjectColor, spawnObjectMass, spawnObjectFixed);
                            break;
                        case 2:
                            particleSystem.AddCylinder(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 5.0f, 2.5f, spawnObjectColor, spawnObjectMass, spawnObjectFixed);
                            break;
                        case 3:
                            particleSystem.AddTorus(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 1.25f, 3.0f, spawnObjectColor, spawnObjectMass, spawnObjectFixed);
                            break;
                        case 4:
                            particleSystem.AddBall(renderer.GetCameraPosition() + renderer.GetCameraOrientation() * 2.0f, renderer.GetCameraOrientation() * 20.0f, 3.0f, spawnObjectColor, spawnObjectMass, spawnObjectFixed);
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
        fluidSpawnDebounce -= DELTA_TIME;
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
