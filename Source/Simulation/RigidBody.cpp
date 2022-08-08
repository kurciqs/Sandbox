#include "Simulation/RigidBody.h"

void RigidBody::RecalculateCOM(const std::vector<Particle*>& particles, bool particlesOrdered) {
    m_centerOfMass = glm::vec3(0.0f);
    m_totalMass = 0.0f;
    if (particlesOrdered) {
        for (Particle *p: particles) {
            m_centerOfMass += p->cpos * p->mass;
            m_totalMass += p->mass;
        }
    }
    else {
        for (int i: m_indices) {
            Particle *p = particles[i];
            m_centerOfMass += p->cpos * p->mass;
            m_totalMass += p->mass;
        }
    }
    m_centerOfMass /= m_totalMass;
}

RigidBody::RigidBody(int begin, int end, const std::vector<Particle *>& particles) {
    m_centerOfMass = glm::vec3(0.0f);
    float totalMass = 0.0f;
    for (int i = begin; i <= end; i++) {
        Particle* p = particles[i];
        m_indices.push_back(i);
        m_centerOfMass += p->cpos * p->mass;
        totalMass += p->mass;
    }
    m_centerOfMass /= totalMass;
    for (int i: m_indices) {
        m_offsets.push_back(m_centerOfMass - particles[i]->cpos); // NOTE: cpos or pos ??
    }

}

glm::vec3 RigidBody::GetRestConfigOffsetToCOM(int index) {
    return m_offsets[index];
}

void RigidBody::AddVertex(const std::vector<Particle*>& particles, int index) {
    m_indices.push_back(index);
    RecalculateCOM(particles, false);
    m_offsets.push_back(m_centerOfMass - particles[index]->cpos);
}

void RigidBody::Draw(const std::vector<Particle*>& particles, Renderer &renderer) {
    for (glm::vec3 off: m_offsets) {
        renderer.DrawLine(m_centerOfMass, m_centerOfMass - off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (int i: m_indices) {
        renderer.DrawLine(particles[i]->cpos, m_centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
}
