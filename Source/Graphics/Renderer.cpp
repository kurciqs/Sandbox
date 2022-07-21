#include "Renderer.h"

namespace FPSCounter {
    static int frames = 0;
    static double t, t0, fps;
    static float printdb = 1.0f;

    void Init() {
        t0 = glfwGetTime();
    }

    void Tick() {
        t = glfwGetTime();

        if((t - t0) > 1.0 || frames == 0)
        {
            fps = (double)frames / (t - t0);
            fps = round(fps);
            t0 = t;
            frames = 0;
        }
        frames++;
        printdb -= 0.01f;
    }

    void Print() {
        if (printdb <= 0.0f) {
            printf("FPS: %.1f\n", fps);
            printdb = 1.0f;
        }
    }
}

static void GLAPIENTRY messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        print_error("OPENGL: %s type = 0x%x, severity = 0x%x, message = %s", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            print_error("Error Code: 0x%8x", err);
        }
    }
}

Renderer::Renderer(Window *window)
:
        m_camera(window, glm::vec3(0.0f, 0.0f, 10.0f), 10.0f),
        m_shader("Resources/Shaders/defaultVert.glsl", "Resources/Shaders/defaultFrag.glsl"),
        m_lineShader("Resources/Shaders/lineVert.glsl", "Resources/Shaders/lineFrag.glsl"),
        m_particleShader("Resources/Shaders/particleVert.glsl", "Resources/Shaders/particleFrag.glsl")
{
    if (!GL_init) {
        print_error("GL not initialized!", 0);
        return;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, nullptr);
    glfwSwapInterval(1);

    // Batching

    m_VAO = VAO();
    m_VBO = VBO();
    m_drawBatch = false;

    m_lineVAO = VAO();
    m_lineVBO = VBO();
    m_drawLineBatch = false;

    // Particles
    m_particleVBO = VBO();
    m_particleVAO = VAO();
    m_drawParticles = false;

    m_quadVBO = VBO();
    std::vector<Position> quadPositions = {
            {glm::vec3(-0.5f, -0.5f, 0.0f)},
            {glm::vec3(0.5f, -0.5f, 0.0f)},
            {glm::vec3(0.5f, 0.5f, 0.0f)},

            {glm::vec3(-0.5, -0.5f, 0.0f)},
            {glm::vec3(0.5, 0.5f, 0.0f)},
            {glm::vec3(-0.5, 0.5f, 0.0f)}
    };
    m_quadVBO.SetData(quadPositions);

    m_particleVAO.Bind();
    m_particleVAO.LinkAttrib(m_quadVBO, 0, 3, GL_FLOAT, sizeof(Position), (void*)0);
    m_particleVAO.Unbind();
}

bool Renderer::InitGlad() {
    if (!gladLoadGL()) {
        print_error("Failed to initialize GLAD!", 0);
        return false;
    }
    GL_init = true;
    return true;
}

void Renderer::Flush() {
    m_batchVertices.clear();
    m_batchLineVertices.clear();
}

void Renderer::UploadVertices() {
    m_VBO.SetData(m_batchVertices);
    m_VAO.Bind();
    m_VAO.LinkAttrib(m_VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_VAO.LinkAttrib(m_VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    m_VAO.Unbind();

    m_lineVBO.SetData(m_batchLineVertices);
    m_lineVAO.Bind();
    m_lineVAO.LinkAttrib(m_lineVBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_lineVAO.LinkAttrib(m_lineVBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    m_lineVAO.Unbind();
}

void Renderer::ClearColor(glm::vec3 color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
}

void Renderer::Clear(int mask) {
    glClear(mask);
}

void Renderer::Render() {
    UploadVertices();

    // Normal:
    if (m_drawBatch) {
        m_shader.Bind();

        m_shader.uploadMat4("model", glm::mat4(1.0f));
        m_camera.UploadCameraMatrix(m_shader, "cam");

        m_VAO.Bind();
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) m_batchVertices.size());
        m_VAO.Unbind();
        m_shader.Unbind();
    }

    // Lines:
    if (m_drawLineBatch) {
        m_lineShader.Bind();
        m_lineVAO.Bind();

        m_lineShader.uploadMat4("model", glm::mat4(1.0f));
        m_camera.UploadCameraMatrix(m_lineShader, "cam");

        glLineWidth(4.0f);
        glDrawArrays(GL_LINES, 0, (GLsizei) m_batchLineVertices.size());
        glLineWidth(1.0f);
        m_lineVAO.Unbind();
        m_lineShader.Unbind();
    }

    Flush();

    if (m_drawParticles) {
        m_particleVAO.Bind();
        m_particleShader.Bind();

        m_particleShader.uploadMat4("model", glm::mat4(1.0f));
        m_camera.UploadProjectionMatrix(m_particleShader, "proj");
        m_camera.UploadViewMatrix(m_particleShader, "view");

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 10);

        m_particleShader.Unbind();
        m_particleVAO.Unbind();
    }

    m_drawBatch = false;
    m_drawLineBatch = false;
    m_drawParticles = false;
}

void Renderer::Update(float dt) {
    m_camera.Inputs(dt);
    m_camera.UpdateMatrix();
}

void Renderer::Shutdown() {
    m_VAO.Delete();
    m_VBO.Delete();
    m_lineVAO.Delete();
    m_lineVBO.Delete();
    m_shader.Delete();
    m_lineShader.Delete();
}

// Shapes:

void Renderer::DrawCube(glm::vec3 position, glm::vec3 size, glm::vec3 color) {
    m_drawBatch = true;
    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});

    m_batchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});

    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});

    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});

    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_batchVertices.push_back({position, color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});

    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_batchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
}

void Renderer::DrawTriangle(glm::vec3 c1, glm::vec3 c2, glm::vec3 c3, glm::vec3 color) {
    m_drawBatch = true;
    m_batchVertices.push_back({c1, color});
    m_batchVertices.push_back({c2, color});
    m_batchVertices.push_back({c3, color});
}

void Renderer::DrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color) {
    m_drawLineBatch = true;

    m_batchLineVertices.push_back({p1, color});
    m_batchLineVertices.push_back({p2, color});
}

void Renderer::DrawParticles(std::vector<Particle*>& particles) {
    m_particleVBO.Bind(); // VBO
    // m_particleVBO.SetData(particles);

    m_particleVAO.Bind(); // VAO

    // TODO :
    /*
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3) + sizeof(float), (void*)0); // aOffset
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3) + sizeof(float), (void*)sizeof(vec3)); // aColor
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vec3) + sizeof(float), (void*)(sizeof(vec3) * 2)); // aColor
    glVertexAttribDivisor(3, 1);
    */

    m_particleVBO.Unbind(); // ~VBO
    m_particleShader.Bind(); // Shader

    m_camera.UploadCameraMatrix(m_particleShader, "cam");
    m_particleShader.uploadMat4("model", glm::mat4(1.0f));

    // essentially makes it be called during Draw()
    m_drawParticles = true;

    m_particleShader.Unbind(); // ~Shader
    m_particleVAO.Unbind(); // ~VAO
}
