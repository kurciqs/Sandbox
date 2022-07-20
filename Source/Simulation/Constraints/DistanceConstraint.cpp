#include "DistanceConstraint.h"

DistanceConstraint::DistanceConstraint(Particle *p1, Particle *p2, float k, float d)
{
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_restDistance = d;
    m_type = ConstraintType::equality;
    m_cardinality = 2;
    m_particles.reserve(2);
    m_particles[0] = p1;
    m_particles[1] = p2;
}

void DistanceConstraint::Project() {
    // TODO: .
}

