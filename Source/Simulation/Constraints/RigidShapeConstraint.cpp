#include "RigidShapeConstraint.h"

RigidShapeConstraint::RigidShapeConstraint(RigidBody* rb, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_rb = rb;
    IncrementCounts();
}

void RigidShapeConstraint::Project() {
    m_rb->UpdateMatrix();
    for (int i = 0; i < m_rb->particles.size(); i++) {
        Particle* p = m_rb->particles[i];

        glm::vec3 target = m_rb->centerOfMass + m_rb->rotation * m_rb->offsets[i];
        if (!p->fixed) p->cpos += SOR_COEF * (target - p->cpos) * m_stiffness / (float)p->num_constraints;
    }
}

void RigidShapeConstraint::Draw(Renderer &renderer) {
    for (glm::vec3 off: m_rb->offsets) {
        renderer.DrawLine(m_rb->centerOfMass, m_rb->centerOfMass - off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (Particle* p: m_particles) {
        renderer.DrawLine(p->pos, m_rb->centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
}

RigidShapeConstraint::~RigidShapeConstraint() {
    DecrementCounts();
}