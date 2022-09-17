#include "RigidShapeConstraint.h"

RigidShapeConstraint::RigidShapeConstraint(RigidBody* rb, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_rb = rb;
    m_particles = m_rb->particles; // maybe too optimal
    IncrementCounts();
}

void RigidShapeConstraint::Project() {
    m_rb->UpdateMatrix();
    for (int i = 0; i < m_rb->particles.size(); i++) {
        Particle* p = m_rb->particles[i];

        glm::vec3 target = m_rb->centerOfMass + m_rb->rotation * m_rb->offsets[i];

        if (!p->fixed) p->cpos += (target - p->cpos) * m_stiffness;
    }
}

void RigidShapeConstraint::Draw(Renderer &renderer) {
    for (glm::vec3 off: m_rb->offsets) {
        renderer.DrawLine(m_rb->centerOfMass, m_rb->centerOfMass + off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (Particle* p: m_rb->particles) {
        renderer.DrawLine(p->cpos, m_rb->centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
    renderer.DrawCube(m_rb->centerOfMass - glm::vec3(0.25), glm::vec3(0.5f), glm::vec3(0.4f, 0.4f, 0.7f));
}

RigidShapeConstraint::~RigidShapeConstraint() {
    DecrementCounts();
}