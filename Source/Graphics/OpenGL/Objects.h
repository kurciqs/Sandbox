#ifndef SANDBOX_OBJECTS_H
#define SANDBOX_OBJECTS_H

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
};

class EBO
{
public:
    explicit EBO(std::vector<GLuint>& indices);

    void Bind() const;
    static void Unbind();
    void Delete();
private:
    GLuint ID{};
};

class VBO
{
public:
    VBO();
    void UploadData(std::vector<Vertex>& vertices) const;
    // TODO: implement UploadData() for different VBO types, mainly the instancing (overloaded)
    void Bind() const;
    void Unbind();
    void Delete();
private:
    GLuint ID{};
};

class VAO
{
public:
    VAO();
    void LinkAttrib(VBO& VBO, GLuint layout, GLint numComponents, GLenum type, GLsizei stride, void* offset) const;
    void Bind();
    void Unbind();
    void Delete();
private:
    GLuint ID{};
    bool m_active = false;
};

#endif