#include "Simulation/RigidBody.h"


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


void RigidBody::RecalculateCOM() {
    centerOfMass = glm::vec3(0.0f);
    totalMass = 0.0f;
    for (Particle *p: particles) {
        centerOfMass += p->cpos * p->mass;
        totalMass += p->mass;
    }
    centerOfMass /= totalMass;
}

void RigidBody::AddVertex(Particle* p, SDFData d, int particleIndex) {
    p->rigidBodyID = ID;
    particles.push_back(p);
    sdfData.insert(std::pair(particleIndex, d));
    // can't add offsets here, COM would be messed up
}

void RigidBody::CalculateOffsets() {
    RecalculateCOM();
    for (Particle *p: particles) {
        offsets.push_back(p->cpos - centerOfMass);
    }
}

void RigidBody::UpdateMatrix() {
    RecalculateCOM();

    glm::mat3 A(0.0f);
    for (int i = 0; i < particles.size(); i++) {
        Particle* pt = particles[i];

        glm::vec3 p = pt->cpos - centerOfMass;
        glm::vec3 q = offsets[i];
        glm::mat3 incr = pt->mass * glm::mat3(p.x * q.x, p.y * q.x, p.z * q.x,
                                            p.x * q.y, p.y * q.y, p.z * q.y,
                                            p.x * q.z, p.y * q.z, p.z * q.z
        ); // m * p * qT

        A += incr;
    }

    glm::mat3 ATA = glm::transpose(A) * A; // A^T * A
    glm::mat3 S = MatrixSolver::matSqrt(ATA); // (sqrt(A^T * A))

    rotation = A * glm::inverse(S);
}

SDFData RigidBody::GetSDFData(int index) {
    return {rotation * sdfData[index].grad, sdfData[index].mag};
}