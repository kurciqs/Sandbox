#include "ContactConstraint.h"

ContactConstraint::ContactConstraint(Particle *p1, Particle *p2, float k) {
    m_stiffness = k;
    m_particles.reserve(2);
    m_particles.push_back(p1);
    m_particles.push_back(p2);
    m_particles[0]->num_constraints++;
    m_particles[1]->num_constraints++;
}

ContactConstraint::~ContactConstraint() {
    m_particles[0]->num_constraints--;
    m_particles[1]->num_constraints--;
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
    if (!p1->fixed) p1->cpos += p1->invMass * delta / (float)p1->num_constraints;
    if (!p2->fixed) p2->cpos -= p2->invMass * delta / (float)p2->num_constraints;
}

void ContactConstraint::Draw(Renderer &renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_particles[1]->cpos, glm::vec3(1.0f, 0.0f, 0.0f));
}
