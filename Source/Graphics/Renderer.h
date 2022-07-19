#ifndef SANDBOX_RENDERER_H
#define SANDBOX_RENDERER_H

#define SANDBOX_USE_OPENGL

#ifdef SANDBOX_USE_OPENGL
#include "OpenGL/Objects.h"
#include "OpenGL/Shader.h"
#include "Camera.h"
#endif

namespace FPSCounter {
    void Init();
    void Tick();
    void Print();
}

static bool GL_init = false;

class Renderer {
public:
    explicit Renderer(Window* window);
    ~Renderer() = default;

    void Update(float dt); // Updates camera
    void Render(); // Draws
    void Shutdown();

    // Shapes:
    void DrawDemo();
    void DrawCube(glm::vec3 position, glm::vec3 size, glm::vec3 color);
    void DrawTriangle();
    void DrawLine();


    static void Clear(int mask);
    static void ClearColor(glm::vec3 color);
    static bool InitGlad();
private:
    void Flush();
    void UploadVertices();

    Camera m_camera;

    VAO m_vao;
    VBO m_vbo;
    Shader m_shader;
    std::vector<Vertex> m_batchVertices;

    VAO m_line_vao;
    VBO m_line_vbo;
    Shader m_line_shader;
    std::vector<Vertex> m_batchLineVertices;
};

#endif //SANDBOX_RENDERER_H
