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

    Camera camera(&window, glm::vec3(0.0f, 0.0f, -2.0f));

    std::vector<Vertex> vertices =
    {
          {glm::vec3(-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
          {glm::vec3(0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
          {glm::vec3(0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)}
    };

    VBO VBO1;
    VBO1.UploadData(vertices);

    VAO VAO1;
    VAO1.Bind();
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    VAO1.Unbind();

    Shader shader("Resources/Shaders/defaultVert.glsl", "Resources/Shaders/defaultFrag.glsl");

    float rot = 0.0f;
    glm::vec4 color(0.2f, 0.3f, 0.3f, 1.0f);
    glm::vec3 pos(0.0f);
    while (!window.shouldClose()) {
        Renderer::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::ClearColor(color);

        camera.Inputs(0.02f);
        camera.Update();

        {
            // Initializes matrices so they are not the null matrix
            glm::mat4 model = glm::mat4(1.0f);

            // Assigns different transformations to each matrix
            model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

            VAO1.Bind();

            shader.Activate();
            shader.uploadMat4("model", model);
            camera.Upload(shader, "cam");

            glDrawArrays(GL_TRIANGLES, 0, 3);

            VAO1.Unbind();
            shader.Deactivate();
        }

        window.checkBasicInput();
        window.pollEvents();
        window.swapBuffers();
    }

    VAO1.Delete();
    VBO1.Delete();
    shader.Delete();

    window.destroy();
    glfwTerminate();

    return 0;
}
