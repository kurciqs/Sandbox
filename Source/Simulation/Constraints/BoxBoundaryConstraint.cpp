#include "BoxBoundaryConstraint.h"

BoxBoundaryConstraint::BoxBoundaryConstraint(Particle *p1, glm::vec3 min, glm::vec3 max, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_min = min;
    m_max = max;
    m_particles.push_back(p1);
    IncrementCounts();
}

BoxBoundaryConstraint::~BoxBoundaryConstraint() {
    DecrementCounts();
}

void BoxBoundaryConstraint::Project() { // TODO: fix this being not so good
    Particle* p1 = m_particles[0];

    glm::vec3 pos = p1->cpos;
    glm::vec3 dir = glm::vec3(
            p1->cpos.x < 0.0f ? glm::max(glm::abs(pos.x - p1->radius) - glm::abs(m_min.x), 0.0f) : glm::min(glm::abs(m_max.x) - glm::abs(pos.x + p1->radius), 0.0f),
            p1->cpos.y < 0.0f ? glm::max(glm::abs(pos.y - p1->radius) - glm::abs(m_min.y), 0.0f) : glm::min(glm::abs(m_max.y) - glm::abs(pos.y + p1->radius), 0.0f),
            p1->cpos.z < 0.0f ? glm::max(glm::abs(pos.z - p1->radius) - glm::abs(m_min.z), 0.0f) : glm::min(glm::abs(m_max.z) - glm::abs(pos.z + p1->radius), 0.0f)
    );

    if (!p1->fixed) p1->cpos += SOR_COEF * dir * m_stiffness / (float)p1->num_constraints;

    //TODO: friction
}

void BoxBoundaryConstraint::Draw(Renderer &renderer) {
    renderer.DrawLineCube(m_min, m_max - m_min, glm::vec3(0.0f, 0.4f, 0.8f));
}
