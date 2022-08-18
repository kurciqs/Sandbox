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

    glm::vec3 n{};
    float d;
    if (dat1.mag < 0 || dat2.mag < 0) {
        glm::vec3 x12 = p2->cpos - p1->cpos;
        float len = glm::length(x12);
        d = diameter - len;
        if (d < EPSILON) return;
        n = x12 / len;
    } else {
        if (dat1.mag < dat2.mag) {
            d = dat1.mag;
            n = dat1.grad;
        }
        else {
            d = dat2.mag;
            n = -dat2.grad;
        }

        if (d < diameter + EPSILON) {
            glm::vec3 x12 = p1->cpos - p2->cpos;
            float len = glm::length(x12);
            d = diameter - len;
            if (d < EPSILON) return;
            x12 = x12 / len;
            float dp = glm::dot(x12, n);
            if (dp < 0.0f) {
                n = x12 - 2.0f * dp * n;
            }
            else {
                n = x12;
            }
        }
    }

    glm::vec3 delta = (1.0f / GetInvMassSum()) * d * n;

    if (!p1->fixed) p1->cpos -= p1->invMass * delta;
    if (!p2->fixed) p2->cpos += p2->invMass * delta;
}

void RigidContactConstraint::Draw(Renderer &renderer) {
    for (int i = 0; i < 2; i++) {
        Particle* p = m_particles[i];
        SDFData s = m_SDFData[i];
        renderer.DrawLine(p->cpos, p->cpos + s.grad * s.mag, glm::vec3(0.3f, 0.8f, 0.2f));
        p->color -= glm::vec3(0.01f);
    }
}
