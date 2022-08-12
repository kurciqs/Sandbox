#ifndef SANDBOX_RENDERER_H
#define SANDBOX_RENDERER_H

#define SANDBOX_USE_OPENGL

#ifdef SANDBOX_USE_OPENGL
#include "OpenGL/Objects.h"
#include "OpenGL/Shader.h"
#include "Camera.h"
#include "Simulation/Particle.h"
#endif

static bool GL_init = false;

class Renderer {
public:
    explicit Renderer(Window* window);
    ~Renderer() = default;

    void Update(float dt); // Updates camera
    void Render(); // Draws
    void Shutdown();

    // Shapes:
    void DrawCube(glm::vec3 position, glm::vec3 size, glm::vec3 color);
    void DrawLineCube(glm::vec3 position, glm::vec3 size, glm::vec3 color);
    void DrawTriangle(glm::vec3 c1, glm::vec3 c2, glm::vec3 c3, glm::vec3 color);
    void DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color);
    void DrawParticles(std::vector<Particle*>& particles);

    static void Clear(int mask);
    static void ClearColor(glm::vec3 color);
    static bool InitGlad();

    glm::vec3 GetCameraPosition() { return m_camera.GetPos(); };
    glm::vec3 GetCameraOrientation() { return m_camera.GetOrientation(); };
private:
    void Flush();
    void UploadVertices();

    Camera m_camera;

    VAO m_VAO;
    VBO m_VBO;
    Shader m_shader;
    std::vector<Vertex> m_batchVertices;
    bool m_drawBatch;

    VAO m_lineVAO;
    VBO m_lineVBO;
    Shader m_lineShader;
    std::vector<Vertex> m_batchLineVertices;
    bool m_drawLineBatch;

    VAO m_particleVAO;
    VBO m_quadVBO;      //
    VBO m_particleVBO;  // +
    Shader m_particleShader;
    bool m_drawParticles;
    GLuint m_numParticles;
};

#endif //SANDBOX_RENDERER_H
