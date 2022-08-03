#include "BoxBoundaryConstraint.h"

BoxBoundaryConstraint::BoxBoundaryConstraint(Particle *p1, glm::vec3 min, glm::vec3 max, float k) {
    m_stiffness = k;
    m_min = min;
    m_max = max;
    m_particles.push_back(p1);
    m_particles[0]->num_constraints++;
}

BoxBoundaryConstraint::~BoxBoundaryConstraint() {
    m_particles[0]->num_constraints--;
}

void BoxBoundaryConstraint::Project() {
    Particle* p1 = m_particles[0];

    // check if particle is in box
    if (p1->cpos.x < m_max.x && p1->cpos.y < m_max.y && p1->cpos.z < m_max.z && p1->cpos.x > m_min.x && p1->cpos.y > m_min.y && p1->cpos.z > m_min.z) {
        return;
    }
    glm::vec3 size = (m_max - m_min) / 2.0f;

    glm::vec3 direction = glm::vec3(glm::max(0.0f, glm::abs(p1->cpos.x) - size.x),
                                    glm::max(0.0f, glm::abs(p1->cpos.y) - size.y),
                                    glm::max(0.0f, glm::abs(p1->cpos.z) - size.z)
                                    );

    if (!p1->fixed) p1->cpos += glm::fastLength(direction) * glm::fastNormalize(direction) * m_stiffness * p1->invMass;
}

void BoxBoundaryConstraint::Draw(Renderer &renderer) {
    renderer.DrawLine(m_min, glm::vec3(m_min.x, m_max.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(m_min, glm::vec3(m_max.x, m_min.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(m_min, glm::vec3(m_min.x, m_min.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(m_max, glm::vec3(m_max.x, m_max.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(m_max, glm::vec3(m_min.x, m_max.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(m_max, glm::vec3(m_max.x, m_min.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_min.x, m_max.y, m_min.z), glm::vec3(m_max.x, m_max.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_min.x, m_max.y, m_min.z), glm::vec3(m_min.x, m_max.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_max.x, m_min.y, m_max.z), glm::vec3(m_max.x, m_min.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_max.x, m_min.y, m_max.z), glm::vec3(m_min.x, m_min.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_max.x, m_min.y, m_min.z), glm::vec3(m_max.x, m_max.y, m_min.z), glm::vec3(0.0f, 0.4f, 0.8f));
    renderer.DrawLine(glm::vec3(m_min.x, m_min.y, m_max.z), glm::vec3(m_min.x, m_max.y, m_max.z), glm::vec3(0.0f, 0.4f, 0.8f));
}
