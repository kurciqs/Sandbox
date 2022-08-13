#include "RigidContactConstraint.h"

// TODO:
RigidContactConstraint::RigidContactConstraint(Particle* p1, Particle* p2, SDFData d1, SDFData d2, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);

    m_particles.push_back(p1);
    m_particles.push_back(p2);
    m_SDFData.push_back(d1);
    m_SDFData.push_back(d2);

    IncrementCounts();
}

RigidContactConstraint::~RigidContactConstraint() {
    DecrementCounts();
}

void RigidContactConstraint::Project() {

}

void RigidContactConstraint::Draw(Renderer &renderer) {
    for (int i = 0; i < 2; i++) {
        Particle* p = m_particles[i];
        SDFData s = m_SDFData[i];
        renderer.DrawLine(p->cpos, p->cpos + s.grad * s.mag, glm::vec3(0.3f, 0.8f, 0.2f));
    }
}
