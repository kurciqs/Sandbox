#ifndef SANDBOX_SHADER_H
#define SANDBOX_SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include "Utils/Utils.h"

#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

std::string get_file_contents(const char* filename);

class Shader
{
public:
    Shader(const char* vertexFile, const char* fragmentFile);
    void Bind();
    void Unbind();
    void Delete() const;

    void UploadVec4(const char* varName, const glm::vec4& vec4) const;
    void UploadVec3(const char* varName, const glm::vec3& vec3) const;
    void UploadVec2(const char* varName, const glm::vec2& vec2) const;
    void UploadIVec4(const char* varName, const glm::ivec4& vec4) const;
    void UploadIVec3(const char* varName, const glm::ivec3& vec3) const;
    void UploadIVec2(const char* varName, const glm::ivec2& vec2) const;
    void UploadFloat(const char* varName, float value) const;
    void UploadInt(const char* varName, int value) const;
    void UploadIntArray(const char* varName, int length, const int* array) const;
    void UploadUInt(const char* varName, unsigned int value) const;
    void UploadBool(const char* varName, bool value) const;
    void UploadMat4(const char* varName, const glm::mat4& mat4) const;
    void UploadMat3(const char* varName, const glm::mat3& mat3) const;

private:
    GLuint ID;
    bool m_active = false;
    static void compileErrors(unsigned int shader, const char* type, const char* filename);
};


#endif //SANDBOX_SHADER_H
