#include "Simulation/RigidBody.h"

void RigidBody::RecalculateCOM(const std::vector<Particle*>& particles) {

}

// TODO:
RigidBody::RigidBody(int begin, int end, const std::vector<Particle *>& particles) {
    for (int i = begin; i < end; i++) {
        m_offsets.push_back(glm::vec3());
    }
}

float RigidBody::DistanceToCOM(glm::vec3 point, int index) { // index for checking
    if (index < 0) {
        return 0.0f;
    }
    else if (index < m_beginIndex || index >= m_lastIndex) {
        print_error("Index %d is out of bounds(%d, %d)", index, m_beginIndex, m_lastIndex);
        return 0.0f;
    }
    return glm::fastDistance(m_centerOfMass, point);
}

float RigidBody::GetRestConfigDistanceToCOM(int index) {
    if (index < 0) {
        return 0.0f;
    }
    else if (index < m_beginIndex || index >= m_lastIndex) {
        print_error("Index %d is out of bounds(%d, %d)", index, m_beginIndex, m_lastIndex);
        return 0.0f;
    }
}
