#include "RigidShapeConstraint.h"


// Helper functions (https://www.juliabloggers.com/jacobi-transformation-of-a-symmetric-matrix/)

float convergence(glm::mat3 A) {
    float res = 0.0f;
    for (int i = 0; i < 3 - 1; i++) {
        for (int j = i + 1; i < 3; i++) {
            res += A[i][j] * A[i][j];
        }
    }
    return res;
}

glm::mat3 randSymmetricMat() {
    glm::mat3 r(1.0f); // just random
    for (int i = 0; i < 3; i++) {
        for (int j = 0; i < 3; i++) {
            r[i][j] = (rand() % 100) / 100.0f;
        }
    }
    return glm::transpose(r) * r;
}

glm::mat3 roundMatrix(glm::mat3 A, float tolerance = 1e-10) {
    glm::mat3 Ap = A;
    //loop through all
    for (int i = 0; i < 3; i++) {
        for (int j = 0; i < 3; i++) {
            if (glm::abs(Ap[i][j]) < tolerance) {
                Ap[i][j] = 0.0f;
            }
        }
    }
    return Ap;
}

glm::mat3 rotateJacobi(glm::mat3 A, int p, int q, glm::mat3* V = nullptr) {
    float theta = (A[q][q] - A[p][p]) / (2.0f * A[p][q]);
    float t = glm::sign(theta) / (glm::abs(theta) + glm::fastSqrt(theta*theta + 1.0f));
    float c = glm::fastInverseSqrt(t*t + 1.0f); // 1 / sqrt(t^2 + 1)
    float s = c * t;
    float tau = s / (1.0f + c);

    glm::vec3 Ap = A[p]; // data arrangement tisms
    glm::vec3 Aq = A[q];
    for (int i = 1; i < 3; i++) {
        A[i][p] = Ap[i] - s * (Aq[i] + tau * Ap[i]);
        // tODO
    }


    if (V) {
        // compute V as well
    }
}

glm::mat3 diagonalize(glm::mat3 A) {
    // TODO
    return glm::mat3(1.0f);
}



// Helper functions ~

RigidShapeConstraint::RigidShapeConstraint(RigidBody *rb, const std::vector<Particle *>& particles, float k) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_rigidBody = rb;

    for (int i: m_rigidBody->m_indices) {
        m_particles.push_back(particles[i]);
    }

    for (Particle* p: m_particles) {
        p->num_constraints++;
    }
}

void RigidShapeConstraint::Project() {
    m_rigidBody->RecalculateCOM(m_particles, true);

    glm::mat3 A(0.0f); // might be tisms
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* pt = m_particles[i];
        glm::vec3 p = pt->cpos - m_rigidBody->GetCOM();
        glm::vec3 q = m_rigidBody->GetRestConfigOffsetToCOM(i);
        A += pt->mass * glm::mat3(p.x*q.x, p.x*q.y, p.x*q.z,
                                  p.y*q.y, p.y*q.y, p.y*q.z,
                                  p.z*q.x, p.z*q.y, p.z*q.z); // m * p * qT
    }

    glm::mat3 ATA = glm::transpose(A) * A; // A^T * A
    glm::mat3 invS = diagonalize(ATA); // (sqrt(A^T * A)) ^ -1

    glm::mat3 rotation = A * invS;

    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p = m_particles[i];

        glm::vec3 target = m_rigidBody->GetCOM() + rotation * m_rigidBody->GetRestConfigOffsetToCOM(i);

        p->cpos += (target - p->cpos) * m_stiffness / (float)p->num_constraints;
    }
}

void RigidShapeConstraint::Draw(Renderer &renderer) {
    for (glm::vec3 off: m_rigidBody->m_offsets) {
        renderer.DrawLine(m_rigidBody->m_centerOfMass, m_rigidBody->m_centerOfMass - off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (Particle* p: m_particles) {
        renderer.DrawLine(p->pos, m_rigidBody->m_centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
}

RigidShapeConstraint::~RigidShapeConstraint() {
    for (Particle* p: m_particles) {
        p->num_constraints--;
    }
}
