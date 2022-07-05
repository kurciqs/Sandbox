#include "Shader.h"

std::string get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

Shader::Shader(const char *vertexFile, const char *fragmentFile) {
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);
    compileErrors(vertexShader, "VERTEX", vertexFile);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT", fragmentFile);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    compileErrors(ID, "PROGRAM", fragmentFile);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Activate() {
    m_active = true;
    glUseProgram(ID);
}

void Shader::Delete() const{

    glDeleteProgram(ID);
}

void Shader::uploadVec4(const char* varName, const glm::vec4& vec4) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform4f(glGetUniformLocation(ID,  varName), vec4.x, vec4.y, vec4.z, vec4.w);
}

void Shader::uploadVec3(const char* varName, const glm::vec3& vec3) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform3f(glGetUniformLocation(ID,  varName), vec3.x, vec3.y, vec3.z);
}

void Shader::uploadVec2(const char* varName, const glm::vec2& vec2) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform2f(glGetUniformLocation(ID,  varName), vec2.x, vec2.y);
}

void Shader::uploadIVec4(const char* varName, const glm::ivec4& vec4) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform4i(glGetUniformLocation(ID,  varName), vec4.x, vec4.y, vec4.z, vec4.w);
}

void Shader::uploadIVec3(const char* varName, const glm::ivec3& vec3) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform3i(glGetUniformLocation(ID,  varName), vec3.x, vec3.y, vec3.z);
}

void Shader::uploadIVec2(const char* varName, const glm::ivec2& vec2) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform2i(glGetUniformLocation(ID,  varName), vec2.x, vec2.y);
}

void Shader::uploadFloat(const char* varName, float value) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform1f(glGetUniformLocation(ID,  varName), value);
}

void Shader::uploadInt(const char* varName, int value) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform1i(glGetUniformLocation(ID,  varName), value);
}

void Shader::uploadUInt(const char* varName, unsigned int value) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform1ui(glGetUniformLocation(ID,  varName), value);
}

void Shader::uploadIntArray(const char* varName, int length, const int* array) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform1iv(glGetUniformLocation(ID,  varName), length, array);
}

void Shader::uploadBool(const char* varName, bool value) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniform1i(glGetUniformLocation(ID,  varName), value ? 1 : 0);
}

void Shader::uploadMat4(const char* varName, const glm::mat4& mat4) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniformMatrix4fv(glGetUniformLocation(ID,  varName), 1, GL_FALSE, glm::value_ptr(mat4));
}

void Shader::uploadMat3(const char* varName, const glm::mat3& mat3) const
{
    if (!m_active)
        print_error("Shader can't upload %s, because it isn't bound!", varName);
    glUniformMatrix3fv(glGetUniformLocation(ID,  varName), 1, GL_FALSE, glm::value_ptr(mat3));
}

void Shader::compileErrors(unsigned int shader, const char *type, const char* filename) {
    GLint hasCompiled;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0)
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            print_error("SHADER_COMPILATION_ERROR for file %s, type %s\n%s\n", filename, type, infoLog);
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            print_error("SHADER_LINKING_ERROR for files %s/.vert, type %s\n%s\n", filename, type, infoLog);
        }
    }
}

void Shader::Deactivate() {
    m_active = false;
    glUseProgram(0);
}


