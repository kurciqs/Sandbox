#include "PositionConstraint.h"

PositionConstraint::PositionConstraint(Particle *p, glm::vec3 pos, float k) {
    m_particles.push_back(p);
    m_pos = pos;
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_particles[0]->num_constraints++;
}

PositionConstraint::~PositionConstraint() {
    m_particles[0]->num_constraints--;
}

void PositionConstraint::Project() {
    Particle* p1 = m_particles[0];

    glm::vec3 dir = p1->cpos - m_pos;

    glm::vec3 delta = -dir * m_stiffness;

    p1->cpos += delta / (float)p1->num_constraints;
}

void PositionConstraint::Draw(Renderer &renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_pos, glm::vec3(0.4f, 0.6f, 0.1f));
}
