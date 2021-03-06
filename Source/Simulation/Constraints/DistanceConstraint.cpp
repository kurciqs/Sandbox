#include "DistanceConstraint.h"

DistanceConstraint::DistanceConstraint(Particle *p1, Particle *p2, float k, float d)
{
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_restDistance = d;
    m_particles.reserve(2);
    m_particles[0] = p1;
    m_particles[1] = p2;
}

void DistanceConstraint::Project() {
    // NOTE: right here I'd send stuff to GPU or somthing and use the results
    Particle* p1 = m_particles[0];
    Particle* p2 = m_particles[1];

    //  p1 - p2
    glm::vec3 diff = p1->cpos - p2->cpos;
    // |p1 − p2|
    float distance = glm::length(diff);

    //  ((|p1 −p2| −d)/w1+w2)* ((p1 - p2)/(|p1 − p2|))
    glm::vec3 correction = ((distance - m_restDistance) / (p1->invMass + p2->invMass)) * (diff / distance) * m_stiffness;

    if (!p1->fixed) p1->cpos -= correction * p1->invMass;
    if (!p2->fixed) p2->cpos += correction * p2->invMass;
}

void DistanceConstraint::Draw(Renderer& renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_particles[1]->cpos, glm::vec3(0.1f, 0.9f, 0.1f));
}
