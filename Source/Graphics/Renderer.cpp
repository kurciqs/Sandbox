#include "Renderer.h"

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
        m_shader("Assets/Shaders/defaultVert.glsl", "Assets/Shaders/defaultFrag.glsl"),
        m_lineShader("Assets/Shaders/lineVert.glsl", "Assets/Shaders/lineFrag.glsl"),
        m_particleShader("Assets/Shaders/particleVert.glsl", "Assets/Shaders/particleFrag.glsl"),
        m_fluidParticleShader("Assets/Shaders/fluidVert.glsl", "Assets/Shaders/fluidFrag.glsl")
{
    if (!GL_init) {
        print_error("GL not initialized!", 0);
        return;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(messageCallback, nullptr);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    m_fluidParticleVBO = VBO();
    m_fluidParticleVAO = VAO();
    m_drawFluidParticles = false;



    m_quadVBO = VBO();
    std::vector<Position> quadPositions = {
            {glm::vec3(-1.0f, -1.0f, 0.0f)},
            {glm::vec3(1.0f, -1.0f, 0.0f)},
            {glm::vec3(1.0f, 1.0f, 0.0f)},

            {glm::vec3(-1.0f, -1.0f, 0.0f)},
            {glm::vec3(1.0f, 1.0f, 0.0f)},
            {glm::vec3(-1.0f, 1.0f, 0.0f)}
    };
    m_quadVBO.SetData<Position>(quadPositions);

    m_particleVAO.Bind();
    m_particleVAO.LinkAttrib(m_quadVBO, 0, 3, GL_FLOAT, sizeof(Position), (void*)0);
    m_particleVAO.Unbind();

    m_fluidParticleVAO.Bind();
    m_fluidParticleVAO.LinkAttrib(m_quadVBO, 0, 3, GL_FLOAT, sizeof(Position), (void*)0);
    m_fluidParticleVAO.Unbind();
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
    for (Vertex v : m_alwaysDrawBatchVertices) {
        m_batchVertices.push_back(v);
    }

    m_VBO.SetData<Vertex>(m_batchVertices);
    m_VAO.Bind();
    m_VAO.LinkAttrib(m_VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_VAO.LinkAttrib(m_VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    m_VAO.Unbind();

    for (Vertex v : m_alwaysDrawBatchLineVertices) {
        m_batchLineVertices.push_back(v);
    }

    m_lineVBO.SetData<Vertex>(m_batchLineVertices);
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

        m_shader.UploadMat4("model", glm::mat4(1.0f));
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

        m_lineShader.UploadMat4("model", glm::mat4(1.0f));
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

        m_particleShader.UploadMat4("model", glm::mat4(1.0f));
        m_camera.UploadProjectionMatrix(m_particleShader, "proj");
        m_camera.UploadViewMatrix(m_particleShader, "view");

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei) m_numParticles);

        m_particleShader.Unbind();
        m_particleVAO.Unbind();
    }

    if (m_drawFluidParticles) {
        // fuor later
    }


    m_numParticles = 0;
    m_drawBatch = false;
    m_drawLineBatch = false;
    m_drawParticles = false;
    m_drawFluidParticles = false;
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

    std::vector<ParticleVertex> vertices;
    std::vector<ParticleVertex> fluidVertices;

    for (auto p : particles) {
        if (m_mode == RenderMode::Circles) {
            vertices.push_back( {p->pos, p->color, p->radius} );
        }
        else {
            // seperate them
            if (p->phase == Phase::Solid) {
                vertices.push_back({p->pos, p->color, p->radius});
            } else if (p->phase == Phase::Liquid) {
                fluidVertices.push_back({p->pos, p->color, p->radius});
            }
        }
    }
    // Normal
    m_numParticles = vertices.size();

    m_particleVBO.Bind(); // VBO
    m_particleVBO.SetData<ParticleVertex>(vertices);
    m_particleVAO.Bind(); // VAO

    m_particleVAO.LinkAttrib(m_particleVBO, 1, 3, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)0);
    m_particleVAO.DivideAttrib(1, 1);
    m_particleVAO.LinkAttrib(m_particleVBO, 2, 3, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)sizeof(glm::vec3));
    m_particleVAO.DivideAttrib(2, 1);
    m_particleVAO.LinkAttrib(m_particleVBO, 3, 1, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)(sizeof(glm::vec3) * 2));
    m_particleVAO.DivideAttrib(3, 1);

    m_particleVBO.Unbind(); // ~VBO

    // essentially makes it be called during Draw()
    m_drawParticles = true;

    m_particleVAO.Unbind(); // ~VAO
    // ~Normal

    // Fluid
    m_numFluidParticles = fluidVertices.size();

    m_fluidParticleVBO.Bind(); // VBO
    m_fluidParticleVBO.SetData<ParticleVertex>(fluidVertices);
    m_fluidParticleVAO.Bind(); // VAO

    m_fluidParticleVAO.LinkAttrib(m_fluidParticleVBO, 1, 3, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)0);
    m_fluidParticleVAO.DivideAttrib(1, 1);
    m_fluidParticleVAO.LinkAttrib(m_fluidParticleVBO, 2, 3, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)sizeof(glm::vec3));
    m_fluidParticleVAO.DivideAttrib(2, 1);
    m_fluidParticleVAO.LinkAttrib(m_fluidParticleVBO, 3, 1, GL_FLOAT, 2 * sizeof(glm::vec3) + sizeof(float), (void*)(sizeof(glm::vec3) * 2));
    m_fluidParticleVAO.DivideAttrib(3, 1);

    m_fluidParticleVBO.Unbind(); // ~VBO

    // essentially makes it be called during Draw()
    m_drawFluidParticles = true;

    m_fluidParticleVAO.Unbind(); // ~VAO
    // ~Fluid


}

void Renderer::DrawLineCube(glm::vec3 position, glm::vec3 size, glm::vec3 color) {
    // lower corner:
    glm::vec3 lc = position;
    // upper corner:
    glm::vec3 uc = position + size;
    DrawLine(lc, glm::vec3(lc.x, uc.y, lc.z), color);
    DrawLine(lc, glm::vec3(uc.x, lc.y, lc.z), color);
    DrawLine(lc, glm::vec3(lc.x, lc.y, uc.z), color);
    DrawLine(uc, glm::vec3(uc.x, uc.y, lc.z), color);
    DrawLine(uc, glm::vec3(lc.x, uc.y, uc.z), color);
    DrawLine(uc, glm::vec3(uc.x, lc.y, uc.z), color);
    DrawLine(glm::vec3(lc.x, uc.y, lc.z), glm::vec3(uc.x, uc.y, lc.z), color);
    DrawLine(glm::vec3(lc.x, uc.y, lc.z), glm::vec3(lc.x, uc.y, uc.z), color);
    DrawLine(glm::vec3(uc.x, lc.y, uc.z), glm::vec3(uc.x, lc.y, lc.z), color);
    DrawLine(glm::vec3(uc.x, lc.y, uc.z), glm::vec3(lc.x, lc.y, uc.z), color);
    DrawLine(glm::vec3(uc.x, lc.y, lc.z), glm::vec3(uc.x, uc.y, lc.z), color);
    DrawLine(glm::vec3(lc.x, lc.y, uc.z), glm::vec3(lc.x, uc.y, uc.z), color);
}

void Renderer::DrawVertices(const std::vector<Vertex>& vertices) {
    for (Vertex v: vertices) {
        m_batchVertices.push_back(v);
    }
}

void Renderer::AlwaysDrawVertices(const std::vector<Vertex> &vertices) {
    for (Vertex v: vertices) {
        m_alwaysDrawBatchVertices.push_back(v);
    }
}

void Renderer::AlwaysDrawTriangle(glm::vec3 c1, glm::vec3 c2, glm::vec3 c3, glm::vec3 color) {
    m_alwaysDrawBatchVertices.push_back({c1, color});
    m_alwaysDrawBatchVertices.push_back({c2, color});
    m_alwaysDrawBatchVertices.push_back({c3, color});
}

void Renderer::AlwaysDrawLine(glm::vec3 p1, glm::vec3 p2, glm::vec3 color) {
    m_alwaysDrawBatchLineVertices.push_back({p1, color});
    m_alwaysDrawBatchLineVertices.push_back({p2, color});
}

void Renderer::AlwaysDrawLineCube(glm::vec3 position, glm::vec3 size, glm::vec3 color) {
    // lower corner:
    glm::vec3 lc = position;
    // upper corner:
    glm::vec3 uc = position + size;
    AlwaysDrawLine(lc, glm::vec3(lc.x, uc.y, lc.z), color);
    AlwaysDrawLine(lc, glm::vec3(uc.x, lc.y, lc.z), color);
    AlwaysDrawLine(lc, glm::vec3(lc.x, lc.y, uc.z), color);
    AlwaysDrawLine(uc, glm::vec3(uc.x, uc.y, lc.z), color);
    AlwaysDrawLine(uc, glm::vec3(lc.x, uc.y, uc.z), color);
    AlwaysDrawLine(uc, glm::vec3(uc.x, lc.y, uc.z), color);
    AlwaysDrawLine(glm::vec3(lc.x, uc.y, lc.z), glm::vec3(uc.x, uc.y, lc.z), color);
    AlwaysDrawLine(glm::vec3(lc.x, uc.y, lc.z), glm::vec3(lc.x, uc.y, uc.z), color);
    AlwaysDrawLine(glm::vec3(uc.x, lc.y, uc.z), glm::vec3(uc.x, lc.y, lc.z), color);
    AlwaysDrawLine(glm::vec3(uc.x, lc.y, uc.z), glm::vec3(lc.x, lc.y, uc.z), color);
    AlwaysDrawLine(glm::vec3(uc.x, lc.y, lc.z), glm::vec3(uc.x, uc.y, lc.z), color);
    AlwaysDrawLine(glm::vec3(lc.x, lc.y, uc.z), glm::vec3(lc.x, uc.y, uc.z), color);
}

void Renderer::AlwaysDrawCube(glm::vec3 position, glm::vec3 size, glm::vec3 color) {
    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});

    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});

    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});

    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});

    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position, color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(0.0f, size.y, size.z), color});

    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, 0.0f, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, 0.0f), color});
    m_alwaysDrawBatchVertices.push_back({position + glm::vec3(size.x, size.y, size.z), color});
}
