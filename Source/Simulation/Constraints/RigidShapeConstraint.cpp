#include "RigidShapeConstraint.h"


RigidShapeConstraint::RigidShapeConstraint(RigidBody *rb, const std::vector<Particle *> particles, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_rigidBody = rb;

    for (int i: m_rigidBody->m_indices) {
        m_particles.push_back(particles[i]);
    }

    for (Particle* p: m_particles) {
        p->num_constraints++;
    }
}

void RigidShapeConstraint::Project() {
    //TODO
}

void RigidShapeConstraint::Draw(Renderer &renderer) {
    for (glm::vec3 off: m_rigidBody->m_offsets) {
        renderer.DrawLine(m_rigidBody->m_centerOfMass, m_rigidBody->m_centerOfMass - off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (Particle* p: m_particles) {
        renderer.DrawLine(p->pos, m_rigidBody->m_centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
}

RigidShapeConstraint::~RigidShapeConstraint() {
    for (Particle* p: m_particles) {
        p->num_constraints--;
    }
}
