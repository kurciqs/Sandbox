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
    m_line_vbo.UploadData(m_batchLineVertices);
}

void Renderer::ClearColor(glm::vec3 color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
}

void Renderer::Clear(int mask) {
    glClear(mask);
}

void Renderer::Render() {

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
