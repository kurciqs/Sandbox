#include "DistanceConstraint.h"

DistanceConstraint::DistanceConstraint(Particle *p1, Particle *p2, float k, float d, float tearDistance)
{
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_restDistance = d;
    m_tearDistance = tearDistance;
    m_particles.reserve(2);
    m_particles.push_back(p1);
    m_particles.push_back(p2);
    IncrementCounts();
}

void DistanceConstraint::Project() {
    if (m_particles.empty()) {
        return;
    }
    // NOTE: right here I'd send stuff to GPU or somthing and use the resulting correction
    Particle* p1 = m_particles[0];
    Particle* p2 = m_particles[1];

    //  p1 - p2
    glm::vec3 diff = p1->cpos - p2->cpos;
    // |p1 − p2|
    float distance = glm::fastLength(diff);

    if (distance > m_tearDistance && m_tearDistance != 0.0f) {
        m_particles.clear();
        return;
    }

    //  ((|p1 −p2| −d)/w1+w2)* ((p1 - p2)/(|p1 − p2|))
    glm::vec3 delta = (distance - m_restDistance) / GetInvMassSum() * (diff / distance) * m_stiffness;

    if (!p1->fixed) p1->cpos += -delta * p1->invMass;
    if (!p2->fixed) p2->cpos += delta * p2->invMass;
}

void DistanceConstraint::Draw(Renderer& renderer) {
    renderer.DrawLine(m_particles[0]->cpos, m_particles[1]->cpos, glm::vec3(0.1f, 0.9f, 0.1f));
}

DistanceConstraint::~DistanceConstraint()  {
    DecrementCounts();
}