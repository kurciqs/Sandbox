#include "BoundaryConstraint.h"

BoundaryConstraint::BoundaryConstraint(Particle *p1, glm::vec3 min, glm::vec3 max, float k) {
    m_stiffness = k;
    m_min = min;
    m_max = max;
    m_particles.push_back(p1);
    m_particles[0]->num_constraints++;
}

BoundaryConstraint::~BoundaryConstraint() {
    m_particles[0]->num_constraints--;
}

void BoundaryConstraint::Project() {

}

void BoundaryConstraint::Draw(Renderer &renderer) {
    renderer.DrawCube(m_min, (m_max - m_min), glm::vec3(0.1f, 0.3f, 0.5f));
}
