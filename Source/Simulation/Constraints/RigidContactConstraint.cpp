#include "RigidContactConstraint.h"

RigidContactConstraint::RigidContactConstraint(Particle* p1, Particle* p2, SDFData d1, SDFData d2, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);

    m_particles.push_back(p1);
    m_particles.push_back(p2);
    m_SDFData.push_back(d1);
    m_SDFData.push_back(d2);

    IncrementCounts();
}

RigidContactConstraint::~RigidContactConstraint() {
    DecrementCounts();
}

void RigidContactConstraint::Project() {
    Particle* p1 = m_particles[0];
    Particle* p2 = m_particles[1];

    glm::vec3 diff = p1->cpos - p2->cpos;
    float diameter = p1->radius + p2->radius;
    if (glm::fastLength(diff) < diameter)
        return;

    SDFData d1 = m_SDFData[0];
    SDFData d2 = m_SDFData[1];

    float absMag1 = glm::abs(d1.mag);
    float absMag2 = glm::abs(d2.mag);

    float mag = glm::min(absMag1, absMag2);
    glm::vec3 normal = absMag1 < absMag2 ? d1.grad : -d2.grad;
    glm::vec3 newNormal(0.0f);

    bool isBorder = absMag1 < diameter || absMag2 < diameter;
    if (isBorder) {
        mag = glm::fastLength(diff) - diameter;
        newNormal = glm::dot(diff, normal) < 0.0f ? diff - 2.0f * glm::dot(diff, normal) * normal : diff;
    }
    else {
        newNormal = normal;
    }

    glm::vec3 delta = 1.0f / GetInvMassSum() * mag * newNormal * m_stiffness;

    if (!p1->fixed) p1->cpos -= p1->invMass * delta / (float)p1->num_constraints;
    if (!p2->fixed) p2->cpos += p2->invMass * delta / (float)p2->num_constraints;
}

void RigidContactConstraint::Draw(Renderer &renderer) {
    for (int i = 0; i < 2; i++) {
        Particle* p = m_particles[i];
        SDFData s = m_SDFData[i];
        renderer.DrawLine(p->cpos, p->cpos + s.grad * s.mag, glm::vec3(0.3f, 0.8f, 0.2f));
        p->color -= glm::vec3(0.01f);
    }
}
