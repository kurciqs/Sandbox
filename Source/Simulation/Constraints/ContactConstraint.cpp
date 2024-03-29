#include "ContactConstraint.h"

ContactConstraint::ContactConstraint(Particle *p1, Particle *p2, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_particles.reserve(2);
    m_particles.push_back(p1);
    m_particles.push_back(p2);
    IncrementCounts();
}

ContactConstraint::~ContactConstraint() {
    DecrementCounts();
}

void ContactConstraint::Project() { // is just like the distance constraint but inequal
    Particle* p1 = m_particles[0];
    Particle* p2 = m_particles[1];

    glm::vec3 normal = p2->cpos - p1->cpos;

    float dist = glm::fastLength(normal);
    float mag = dist - (p1->radius + p2->radius);

    if (mag > 0.0f) // inequality constraint
        return;

    glm::vec3 delta = (mag / GetInvMassSum()) * (normal / dist) * m_stiffness;

    if (!p1->fixed) p1->cpos += p1->invMass * delta;
    if (!p2->fixed) p2->cpos -= p2->invMass * delta;
}

void ContactConstraint::Draw(Renderer &renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_particles[1]->cpos, glm::vec3(1.0f, 0.0f, 0.0f));
}
