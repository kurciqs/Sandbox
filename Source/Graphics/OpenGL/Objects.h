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
    GLuint ID{};
    explicit EBO(std::vector<GLuint>& indices);

    void Bind();
    void Unbind();
    void Delete();
};

class VBO
{
public:
    GLuint ID{};
    VBO();
    void UploadData(std::vector<Vertex>& vertices);

    void Bind();
    void Unbind();
    void Delete();
};

class VAO
{
public:
    GLuint ID{};
    VAO();

    void LinkAttrib(VBO& VBO, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
    void Bind();
    void Unbind();
    void Delete();
};

#endif