#include "Camera.h"

Camera::Camera(Window* window, glm::vec3 position, float speed) : m_pos(position), m_window(window), m_speed(speed) {}

void Camera::UpdateMatrix() {
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    view = glm::lookAt(m_pos, m_pos + m_orientation, up);

    projection = glm::perspective(glm::radians(m_fov), m_window->GetAspectRatio(), 0.05f, 1000.0f);
    m_cameraMatrix = projection * view;
}

void Camera::Upload(Shader &shader, const char *uniform_name) {
    shader.uploadMat4(uniform_name, m_cameraMatrix);
}

void Camera::Inputs(float dt) {
    // Keyboard

    auto mvForce = glm::vec3(0.0f);
    glm::vec3 horizontal = {m_orientation.x, 0.0f, m_orientation.z};
    if (Input::isKeyDown(GLFW_KEY_W)) {
        mvForce += (glm::normalize(horizontal) * m_speed);
    }
    if (Input::isKeyDown(GLFW_KEY_S)) {
        mvForce -= (glm::normalize(horizontal) * m_speed);
    }
    if (Input::isKeyDown(GLFW_KEY_A)) {
        mvForce -= glm::normalize(glm::cross(horizontal, up)) * m_speed;
    }
    if (Input::isKeyDown(GLFW_KEY_D)) {
        mvForce += glm::normalize(glm::cross(horizontal, up)) * m_speed;
    }
    if (Input::isKeyDown(GLFW_KEY_SPACE)) {
        mvForce += (up * m_speed);
    }
    if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT)) {
        mvForce -= (up * m_speed);
    }
    ApplyForce(mvForce);

    // ~Keyboard

    // Controller
    m_pos += m_vel * dt;
    m_vel += m_acc * dt;
    m_vel *= 0.95f;
    m_acc = glm::vec3(0.0f);

    if (Input::isKeyDown(GLFW_KEY_Q)) {
        // print the position of the camera
        printf("\n--------------------------- Camera data ---------------------------\n");
        printf("Position: %f %f %f\n", m_pos.x, m_pos.y, m_pos.z);
        // print the forward vector of the camera
        printf("Forward: %f %f %f\n", m_orientation.x,m_orientation.y, m_orientation.z);
        printf("-------------------------------------------------------------------\n\n");
    }
    // ~Controller

    // Mouse
    if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        m_window->HideCursor();

        if (firstClick)
        {
            m_window->SetCursorPos( {m_window->m_width / 2, m_window->m_height / 2} );
            firstClick = false;
        }

        glm::ivec2 mouse = m_window->GetCursorPos();

        float rotX = m_sensitivity * (float)((float)mouse.y - (float)m_window->m_height / 2.0f) / (float)m_window->m_height;
        float rotY = m_sensitivity * (float)((float)mouse.x - (float)m_window->m_width / 2.0f) / (float)m_window->m_width;

        glm::vec3 newOrientation = glm::rotate(m_orientation, glm::radians(-rotX), glm::normalize(glm::cross(m_orientation, up)));

        if (abs(glm::angle(newOrientation, up) - glm::radians(90.0f)) <= glm::radians(85.0f))
        {
            m_orientation = newOrientation;
        }

        m_orientation = glm::rotate(m_orientation, glm::radians(-rotY), up);

        m_window->SetCursorPos( {m_window->m_width / 2, m_window->m_height / 2} );
    }

    else if (Input::isMouseButtonUp(GLFW_MOUSE_BUTTON_LEFT))
    {
        m_window->ShowCursor();
        firstClick = true;
    }
    // ~Mouse
}

void Camera::ApplyForce(glm::vec3 force) {
    if (glm::isnan(force.x) || glm::isnan(force.y) || glm::isnan(force.z)) {
        return;
    }
    m_acc += force;
}