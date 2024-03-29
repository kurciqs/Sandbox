#ifndef SANDBOX_CAMERA_H
#define SANDBOX_CAMERA_H
#include "OpenGL/Shader.h"
#include "Window/Window.h"

class Camera {
public:
    Camera(Window* window, glm::vec3 position, float speed);
    ~Camera() = default;
    void UpdateMatrix();
    void UploadCameraMatrix(Shader& shader, const char* uniform_name);
    void UploadProjectionMatrix(Shader& shader, const char* uniform_name);
    void UploadViewMatrix(Shader& shader, const char* uniform_name);
    void Inputs(float dt);
    glm::vec3 GetPos() { return m_pos; };
    glm::vec3 GetOrientation() { return m_orientation; };
private:
    void ApplyForce(glm::vec3 force);
    Window* m_window;
    glm::vec3 m_pos;
    glm::vec3 m_vel{0.0f, 0.0f, 0.0f};
    glm::vec3 m_acc{0.0f, 0.0f, 0.0f};
    float m_speed = 0.5f;
    float m_fov = 45.0f;
    float m_sensitivity = 225.0f;
    glm::mat4 m_cameraMatrix = glm::mat4(1.0f);
    glm::mat4 m_viewMatrix = glm::mat4(1.0f);
    glm::mat4 m_projectionMatrix = glm::mat4(1.0f);
    glm::vec3 m_orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    bool firstClick = true;
};

#endif //SANDBOX_CAMERA_H
