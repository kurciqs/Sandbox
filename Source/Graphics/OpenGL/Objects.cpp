#include "Objects.h"


EBO::EBO(std::vector<GLuint>& indices)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr) (indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);
}


void EBO::Bind()
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


VAO::VAO()
{
    glGenVertexArrays(1, &ID);
}


void VAO::LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset)
{
    VBO.Bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    VBO.Unbind();
}


void VAO::Bind()
{
    glBindVertexArray(ID);
}


void VAO::Unbind()
{
    glBindVertexArray(0);
}


void VAO::Delete()
{
    glDeleteVertexArrays(1, &ID);
}

void VBO::Bind()
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

void VBO::UploadData(std::vector<Vertex> &vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
