#include "Objects.h"
#include "Utils/Utils.h"

// EBO
EBO::EBO(std::vector<GLuint>& indices)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);
}

void EBO::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}


void EBO::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete()
{
    glDeleteBuffers(1, &ID);
}
// ~EBO

// VAO
VAO::VAO()
{
    glGenVertexArrays(1, &ID);
}

void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLint numComponents, GLenum type, GLsizei stride, void* offset) const {
    if (!m_active)
    {
        print_error("VAO not active, unable to link attribute!", 0);
        return;
    }

    VBO.Bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO.Unbind();
}

void VAO::Bind()
{
    m_active = true;
    glBindVertexArray(ID);
}

void VAO::Unbind()
{
    m_active = false;
    glBindVertexArray(0);
}

void VAO::Delete()
{
    glDeleteVertexArrays(1, &ID);
}

void VAO::DivideAttrib(GLuint n, GLuint m) const {
    if (!m_active)
    {
        print_error("VAO not active, unable to divide attribute!", 0);
        return;
    }
    glVertexAttribDivisor(n, m);
}
// ~VAO

// VBO
void VBO::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete()
{
    glDeleteBuffers(1, &ID);
}

VBO::VBO() {
    glGenBuffers(1, &ID);
}

// ~VBO