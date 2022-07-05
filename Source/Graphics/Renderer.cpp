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
m_camera(window, glm::vec3(0.0f, 0.0f, -2.0f)),
m_shader("Resources/Shaders/defaultVert.glsl", "Resources/Shaders/defaultFrag.glsl"),
m_line_shader("Resources/Shaders/lineVert.glsl", "Resources/Shaders/lineFrag.glsl")
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

    m_vao = VAO();
    m_vbo = VBO();
    m_line_vao = VAO();
    m_line_vbo = VBO();
}

bool Renderer::InitGL() {
    if (!gladLoadGL()) {
        print_error("Failed to initialize GLAD!", 0);
        return false;
    }
    GL_init = true;
    return true;
}

Renderer::~Renderer() {

}

void Renderer::Flush() {
    m_batchVertices.clear();
    m_batchLineVertices.clear();
}

void Renderer::UploadVertices() {
    m_vbo.UploadData(m_batchVertices);
    m_vao.Bind();
    m_vao.LinkAttrib(m_vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_vao.LinkAttrib(m_vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    m_vao.Unbind();

    m_line_vbo.UploadData(m_batchLineVertices);
    m_line_vao.Bind();
    m_line_vao.LinkAttrib(m_line_vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_line_vao.LinkAttrib(m_line_vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
    m_line_vao.Unbind();
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
    m_shader.Activate();

    m_shader.uploadMat4("model", glm::mat4(1.0f));
    m_camera.Upload(m_shader, "cam");

    m_vao.Bind();
    glDrawArrays(GL_TRIANGLES, 0, m_batchVertices.size());
    m_vao.Unbind();
    m_shader.Deactivate();

    // Lines:
    m_line_shader.Activate();
    m_line_vao.Bind();

    m_line_shader.uploadMat4("model", glm::mat4(1.0f));
    m_camera.Upload(m_line_shader, "cam");

    glDrawArrays(GL_LINES, 0, m_batchLineVertices.size());
    m_line_vao.Unbind();
    m_line_shader.Deactivate();

    Flush();
}

void Renderer::Update(float dt) {
    m_camera.Inputs(dt);
    m_camera.Update();
}

void Renderer::Shutdown() {
    m_vao.Delete();
    m_vbo.Delete();
    m_line_vao.Delete();
    m_line_vbo.Delete();
    m_shader.Delete();
    m_line_shader.Delete();
}

void Renderer::DrawDemo() {
    std::vector<Vertex> vertices =
    {
            {glm::vec3(-0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
            {glm::vec3(0.5f, -0.5f * float(sqrt(3)) / 3, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
            {glm::vec3(0.0f, 0.5f * float(sqrt(3)) * 2 / 3, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)}
    };

    for (auto & vertex : vertices) {
        m_batchVertices.push_back(vertex);
    }
}
