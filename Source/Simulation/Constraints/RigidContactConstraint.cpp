#include "RigidContactConstraint.h"

// TODO:
RigidContactConstraint::RigidContactConstraint(RigidBody *rb1, RigidBody *rb2, Particle* p1, Particle* p2, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);

    m_particles.push_back(p1);
    m_particles.push_back(p2);

    m_rigidBodies.push_back(rb1);
    m_rigidBodies.push_back(rb2);
//    IncrementCounts();
}

RigidContactConstraint::~RigidContactConstraint() {
//    DecrementCounts();
}

void RigidContactConstraint::Project() {

}

void RigidContactConstraint::Draw(Renderer &renderer) {

}
