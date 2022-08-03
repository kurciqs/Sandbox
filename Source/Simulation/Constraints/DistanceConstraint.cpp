#include "DistanceConstraint.h"

DistanceConstraint::DistanceConstraint(Particle *p1, Particle *p2, float k, float d)
{
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_restDistance = d;
    m_particles.reserve(2);
    m_particles.push_back(p1);
    m_particles.push_back(p2);
    m_particles[0]->num_constraints++;
    m_particles[1]->num_constraints++;
}

void DistanceConstraint::Project() {
    // NOTE: right here I'd send stuff to GPU or somthing and use the resulting correction
    Particle* p1 = m_particles[0];
    Particle* p2 = m_particles[1];

    //  p1 - p2
    glm::vec3 diff = p1->cpos - p2->cpos;
    // |p1 − p2|
    float distance = glm::fastLength(diff);

    //  ((|p1 −p2| −d)/w1+w2)* ((p1 - p2)/(|p1 − p2|))
    glm::vec3 delta = (distance - m_restDistance) / GetInvMassSum() * (diff / distance) * m_stiffness;

    if (!p1->fixed) p1->cpos += -delta * p1->invMass / (float)p1->num_constraints;
    if (!p2->fixed) p2->cpos += delta * p2->invMass / (float)p2->num_constraints;
}

void DistanceConstraint::Draw(Renderer& renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_particles[1]->cpos, glm::vec3(0.1f, 0.9f, 0.1f));
}

DistanceConstraint::~DistanceConstraint()  {
    m_particles[0]->num_constraints--;
    m_particles[1]->num_constraints--;
}