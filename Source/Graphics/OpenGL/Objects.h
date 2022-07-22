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

struct Position
{
    glm::vec3 position;
};

struct ParticleVertex {
    glm::vec3 position;
    glm::vec3 color;
    float radius;
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

    template<typename T>
    void SetData(std::vector<T> &ts) const {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
        glBufferData(GL_ARRAY_BUFFER, (GLsizei)(ts.size() * sizeof(T)), ts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

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
    void DivideAttrib(GLuint n, GLuint m) const;
    void Bind();
    void Unbind();
    void Delete();
private:
    GLuint ID{};
    bool m_active = false;
};

#endif