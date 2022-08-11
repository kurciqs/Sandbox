#include "RigidShapeConstraint.h"


// Helper functions https://github.com/albi3ro/M4/blob/ccd27d4b8b24861e22fe806ebaecef70915081a8/Numerics_Prog/Jacobi-Transformation.ipynb

glm::mat3 roundMatrix(glm::mat3 A, float tolerance = 1e-10) {
    glm::mat3 Ap = A;
    //loop through all
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (glm::abs(Ap[i][j]) < tolerance) {
                Ap[i][j] = 0.0f;
            }
        }
    }
    return Ap;
}

glm::dmat3 rotateJacobi(glm::dmat3 A, int p, int q, glm::dmat3* V = nullptr) {
    if (!A[q][p])
        return A;
    double theta = (A[q][q] - A[p][p]) / (2.0f * A[q][p]);
    double t = glm::sign(theta) / (glm::abs(theta) + glm::fastSqrt(theta*theta + 1.0f));

    double c = 1.0f / glm::sqrt(t*t + 1.0f); // 1 / sqrt(t^2 + 1)
    double s = c * t;
    double tau = s / (1.0f + c);

    glm::dvec3 Ap = A[p]; // data arrangement tisms
    glm::dvec3 Aq = A[q];
    for (int i = 0; i < 3; i++) {
        A[p][i] = Ap[i] - s * (Aq[i] + tau * Ap[i]);
        A[q][i] = Aq[i] + s * (Ap[i] - tau * Aq[i]);

        A[i][p] = A[p][i];
        A[i][q] = A[q][i];
    }
    A[p][q] = 0.0f;
    A[q][p] = 0.0f;
    A[p][p] = Ap[p] - t * Aq[p];
    A[q][q] = Aq[q] + t * Aq[p];

    if (V) {
        // compute V as well
        glm::vec3 Vp = (*V)[p];
        glm::vec3 Vq = (*V)[q];
        for (int i = 0; i < 3; i++) {
            (*V)[p][i] = c * Vp[i] - s * Vq[i];
            (*V)[q][i] = s * Vp[i] + c * Vq[i];
        }
    }

    return A;
}

glm::dmat3 sweepJacobi(glm::dmat3 A, glm::dmat3* V = nullptr) {
    A = rotateJacobi(A, 1, 0, V);
    A = rotateJacobi(A, 2, 0, V);
    A = rotateJacobi(A, 2, 1, V);

    return A;
}

glm::mat3 matSqrt(glm::dmat3 inA) {
    glm::dmat3 A0 = inA;
    glm::dmat3 V(1.0f);
    glm::dmat3 diagonal = sweepJacobi(inA, &V);
    for (int i = 0; i < 9; i++) {
        diagonal = sweepJacobi(diagonal, &V);
    }

    glm::dmat3 sqrtDiag(1.0f);
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            sqrtDiag[i][j] = glm::sqrt(diagonal[i][j]);
        }
    }

    glm::mat3 res = V * sqrtDiag * glm::inverse(V);

    return res;
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

        glm::vec3 p = pt->cpos - m_rigidBody->m_centerOfMass;
        glm::vec3 q = m_rigidBody->m_offsets[i];

        A += pt->mass * glm::mat3(p.x*q.x, p.x*q.y, p.x*q.z,
                                  p.y*q.x, p.y*q.y, p.y*q.z,
                                  p.z*q.x, p.z*q.y, p.z*q.z
                                  ); // m * p * qT
    }

    glm::mat3 ATA = glm::transpose(A) * A; // A^T * A
    glm::mat3 S = roundMatrix(matSqrt(ATA)); // (sqrt(A^T * A))

    glm::mat3 rotation = A * glm::inverse(S);
//    printMat3(rotation);

    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p = m_particles[i];

        glm::vec3 target = m_rigidBody->m_centerOfMass + rotation * m_rigidBody->m_offsets[i];
        p->cpos += (target - p->cpos) * m_stiffness / (float)p->num_constraints;


        m_rigidBody->m_offsets[i] = rotation * m_rigidBody->m_offsets[i];

    }
    printf("\n%f %f %f\n\n", m_rigidBody->m_offsets[0].x, m_rigidBody->m_offsets[0].y, m_rigidBody->m_offsets[0].z);
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
