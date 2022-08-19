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
    Particle *p1 = m_particles[0], *p2 = m_particles[1];
    SDFData dat1 = m_SDFData[0], dat2 = m_SDFData[1];

    float diameter = p1->radius + p2->radius;

    glm::vec3 contactNormal{};
    float mag;
    glm::vec3 diff = p2->cpos - p1->cpos;
    if (dat1.mag < 0 || dat2.mag < 0) {
        glm::vec3 x12 = diff;
        float len = glm::length(x12);
        mag = diameter - len;
        if (mag < EPSILON) return;
        contactNormal = x12 / len;
    }
    else {
        if (dat1.mag < dat2.mag) {
            mag = dat1.mag;
            contactNormal = dat1.grad;
        }
        else {
            mag = dat2.mag;
            contactNormal = -dat2.grad;
        }

        if (mag < diameter + EPSILON) {
            glm::vec3 x12 = -diff;
            float len = glm::length(x12);
            mag = diameter - len;
            if (mag < EPSILON) return;
            x12 = x12 / len;
            float dp = glm::dot(x12, contactNormal);
            if (dp < 0.0f) {
                contactNormal = x12 - 2.0f * dp * contactNormal;
            }
            else {
                contactNormal = x12;
            }
        }
    }

    float oneOverWSum = 1.0f / GetInvMassSum();
    glm::vec3 dlt = oneOverWSum * mag * contactNormal;

    if (!p1->fixed) p1->cpos -= dlt * p1->invMass / (float)p1->num_constraints;
    if (!p2->fixed) p2->cpos += dlt * p2->invMass / (float)p2->num_constraints;

    glm::vec3 frictionNormal = glm::normalize(contactNormal);
    glm::vec3 velocityDiff = (p1->cpos - p1->pos) - (p2->cpos - p2->pos);
    glm::vec3 frictionDelta = velocityDiff - glm::dot(velocityDiff, frictionNormal) * frictionNormal;
    float frictionDeltaLength = glm::length2(frictionDelta);
    if (frictionDeltaLength < EPSILON * EPSILON) {
        return;
    }

    if (frictionDeltaLength < STATIC_FRICTION_COEF * mag * STATIC_FRICTION_COEF * mag) {
        if (!p1->fixed) p1->cpos -= frictionDelta * p1->invMass * oneOverWSum;
        if (!p2->fixed) p2->cpos += frictionDelta * p2->invMass * oneOverWSum;
    }
    else {
        glm::vec3 newDelta = frictionDelta * glm::min(KINETIC_FRICTION_COEF * mag / frictionDeltaLength, 1.0f);
        if (!p1->fixed) p1->cpos -= newDelta * p1->invMass * oneOverWSum;
        if (!p2->fixed) p2->cpos += newDelta * p2->invMass * oneOverWSum;
    }

}

void RigidContactConstraint::Draw(Renderer &renderer) {
    for (int i = 0; i < 2; i++) {
        Particle* p = m_particles[i];
        SDFData s = m_SDFData[i];
        renderer.DrawLine(p->cpos, p->cpos + s.grad, glm::vec3(0.9f, 0.0f, 0.2f));
    }
}
