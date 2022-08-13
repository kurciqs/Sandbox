#include "RigidShapeConstraint.h"

// Helper functions https://github.com/albi3ro/M4/blob/ccd27d4b8b24861e22fe806ebaecef70915081a8/Numerics_Prog/Jacobi-Transformation.ipynb
namespace MatrixSolver {
    glm::mat3 roundMatrix(glm::mat3 A, float tolerance = 0.0001f) {
        glm::mat3 Ap = A;
        //loop through all
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                Ap[i][j] = glm::abs(Ap[i][j]) < tolerance ? 0.0f : Ap[i][j];
            }
        }
        return Ap;
    }

    glm::mat3 rotateJacobi(glm::mat3 A, int p, int q, glm::mat3 *V = nullptr) {
        if (A[q][p] == 0) {
            return A;
        }

        float theta = (A[q][q] - A[p][p]) / (2.0f * A[q][p]);
        float t = glm::sign(theta) / (glm::abs(theta) + glm::fastSqrt(theta * theta + 1.0f));

        float c = glm::fastInverseSqrt(t * t + 1.0f);
        float s = c * t;
        float tau = s / (1.0f + c);

        glm::vec3 Ap = A[p];
        glm::vec3 Aq = A[q];
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

    glm::mat3 sweepJacobi(glm::mat3 A, glm::mat3 *V = nullptr) {
        A = rotateJacobi(A, 1, 0, V);
        A = rotateJacobi(A, 2, 0, V);
        A = rotateJacobi(A, 2, 1, V);

        return A;
    }

    glm::mat3 matSqrt(glm::mat3 inA) {
        glm::mat3 V(1.0f);
        glm::mat3 diagonal = sweepJacobi(inA, &V);
        for (int i = 0; i < 7; i++) {
            diagonal = sweepJacobi(diagonal, &V);
        }

        glm::mat3 sqrtDiag(1.0f);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sqrtDiag[i][j] = glm::sqrt(diagonal[i][j]);
            }
        }

        glm::mat3 res = V * sqrtDiag * glm::inverse(V);

        return res;
    }
}
// Helper functions ~

RigidShapeConstraint::RigidShapeConstraint(int rigidBodyID, int begin, int end, const std::vector<Particle *>& particles, float k) {
    ID = rigidBodyID;
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);

    for (int i = begin; i <= end; i++) {
        Particle* p = particles[i];
        if (p->rigidBodyID != ID) {
            print_error("IDs do not match: rigidBody %d - %d", ID, p->rigidBodyID);
            continue;
        }
        m_particles.push_back(p);
        m_centerOfMass += p->cpos * p->mass;
        m_totalMass += p->mass;
    }
    m_centerOfMass /= m_totalMass;

    for (Particle* p : m_particles) {
        m_offsets.push_back(p->cpos - m_centerOfMass);
    }

    IncrementCounts();
}

void RigidShapeConstraint::Project() {
    RecalculateCOM();

    glm::mat3 A(0.0f); // might be tisms
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* pt = m_particles[i];

        glm::vec3 p = pt->cpos - m_centerOfMass;
        glm::vec3 q = m_offsets[i];

        A += pt->mass * glm::mat3(p.x * q.x, p.y * q.x, p.z * q.x,
                                  p.x * q.y, p.y * q.y, p.z * q.y,
                                  p.x * q.z, p.y * q.z, p.z * q.z
                                  ); // m * p * qT
    }

    glm::mat3 ATA = glm::transpose(A) * A; // A^T * A
    glm::mat3 S = MatrixSolver::matSqrt(ATA); // (sqrt(A^T * A))

    glm::mat3 rotation = A * glm::inverse(S);

    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p = m_particles[i];

        glm::vec3 target = m_centerOfMass + rotation * m_offsets[i];
        if (!p->fixed) p->cpos += SOR_COEF * (target - p->cpos) * m_stiffness / (float)p->num_constraints;
    }
}

void RigidShapeConstraint::Draw(Renderer &renderer) {
    for (glm::vec3 off: m_offsets) {
        renderer.DrawLine(m_centerOfMass, m_centerOfMass - off, glm::vec3(0.0f, 0.7f, 0.1f));
    }
    for (Particle* p: m_particles) {
        renderer.DrawLine(p->pos, m_centerOfMass, glm::vec3(0.6f, 0.7f, 0.1f));
    }
}

RigidShapeConstraint::~RigidShapeConstraint() {
    DecrementCounts();
}

void RigidShapeConstraint::RecalculateCOM() {
    m_centerOfMass = glm::vec3(0.0f);
    m_totalMass = 0.0f;
    for (Particle *p: m_particles) {
        m_centerOfMass += p->cpos * p->mass;
        m_totalMass += p->mass;
    }
    m_centerOfMass /= m_totalMass;
}

void RigidShapeConstraint::AddParticle(Particle* p) {
    m_particles.push_back(p);
    RecalculateCOM();
    m_offsets.push_back(p->cpos - m_centerOfMass);
}