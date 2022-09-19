#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(int ID, std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density, float viscosityMag) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;
    m_viscosityMag = viscosityMag;
    m_ID = ID;

    m_allParticles = &allParticles;
    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        m_allParticles->at(i)->num_constraints++;
        m_neighbours.emplace_back(); // empty vector
        m_lambdas.emplace(i, 0.0f);
        m_deltas.emplace_back(0.0f);
    }
}

FluidConstraint::~FluidConstraint() {
    for (int i : m_particleIndices) {
        m_allParticles->at(i)->num_constraints--;
    }
}

void FluidConstraint::Project() {
    // find neighbours
    for (int ind = 0; ind < m_particleIndices.size(); ind++) {
        m_neighbours[ind].clear();
        int i = m_particleIndices[ind];
        Particle* p_i = m_allParticles->at(i);
        for (int j = 0; j < m_allParticles->size(); j++) {
            Particle* p_j = m_allParticles->at(j);
            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float d2 = glm::dot(diff, diff);
            if (d2 < H2) {
                m_neighbours[ind].push_back(j);
            }
        }
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle* p_i = m_allParticles->at(i);

        float density_i = 0.0f;
        float denom_i = 0.0f;
        for (int j: m_neighbours[k]) {
            Particle* p_j = m_allParticles->at(j);

            glm::vec3 r = p_i->cpos - p_j->cpos;
            density_i += p_j->mass * poly6(r);

            glm::vec3 gr = grad(k, j);
            denom_i += glm::dot(gr, gr) * p_j->invMass;
        }


        m_lambdas[i] = -((density_i / m_density) - 1.0f) / (denom_i + EPSILON_RELAX);
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle* p_i = m_allParticles->at(i);
        float lambda_i = m_lambdas[i];

        const float corrW = poly6(glm::vec3(MAG_Q_CORR, 0.0f, 0.0f) * H);
        glm::vec3 corr(0.0f);
        for (int j: m_neighbours[k]) {
            Particle* p_j = m_allParticles->at(j);
            float lambda_j = m_lambdas[j];
            glm::vec3 diff = p_i->cpos - p_j->cpos;

            float scorr = -K_CORR * glm::pow(poly6(diff) / corrW, N_CORR);
            corr += p_j->mass * (lambda_i + lambda_j + scorr) * spikyGrad(diff);
        }

        m_deltas[k] = corr / m_density;
    }

        // viscosity / vorticity

   /* for (int localInd = 0; localInd < m_particleIndices.size(); localInd++) {
        glm::vec3 corr(0.0f);
        int i = m_particleIndices[localInd];  // fluid particle index (global)

        glm::vec3 omega(0.0f);
        glm::vec3 viscosity(0.0f);
        Particle *p_i = m_allParticles->at(i);

        for (int localInd2 = 0; localInd2 < m_neighbours[localInd].size(); localInd2++) {
            int j = m_neighbours[localInd][localInd2];  // index to localInd's neighbour number localInd2 (global)

            if (i == j) continue;

            Particle *p_j = m_allParticles->at(j);
            if (p_j->phase == Phase::Liquid) {
                glm::vec3 velDiff = ((p_j->cpos - p_j->pos) - (p_i->cpos - p_i->pos));
                glm::vec3 posDiff = p_i->cpos - p_j->cpos;
                float posDiffLength = glm::length(posDiff);

                omega += glm::cross(velDiff, spikyGrad(posDiff, posDiffLength));

                viscosity += velDiff * poly6(posDiffLength);
            }
        }

//        p_i->viscosity = viscosity * m_viscosityMag;

        float omegaLength = glm::fastLength(omega);
        if (omegaLength == 0.0f) {
            return;
        }

        glm::vec3 eta(0.0f);
        for (int localInd2 = 0; localInd2 < m_neighbours[localInd].size(); localInd2++) {
            int j = m_neighbours[localInd][localInd2];  // index to localInd's neighbour number localInd2 (global)
            Particle *p_j = m_allParticles->at(j);
            if (p_j->phase == Phase::Liquid) {
                glm::vec3 diff = p_i->cpos - p_j->cpos;
                float l = glm::fastLength(diff);
                eta += spikyGrad(diff, l) * omegaLength;
            }
        }

        if (eta.x == 0.0f && eta.y == 0.0f && eta.z == 0.0f) {
            return;
        }
        glm::vec3 N = glm::fastNormalize(eta);
//        p_i->ApplyForce(VORTICITY_COEF * (glm::cross(N, eta)));
    }*/
}

void FluidConstraint::Draw(Renderer &renderer) {
    printf("%d  %d\n", (int)m_particleIndices.size(), (int)m_allParticles->size());
}

float FluidConstraint::poly6(const glm::vec3& r) {
    float rlen = glm::length(r);
    if (rlen > H || rlen == 0.0f) {
        return 0.0f;
    }

    float term2 = (H2 - rlen * rlen);
    return k_Poly6 * (term2 * term2 * term2);
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r) {
    float rlen = glm::length(r);
    if (rlen > H) {
        return glm::vec3(0.0f);
    }
    float coeff = (H - rlen) * (H - rlen) * k_SpikyGrad;
    coeff /= glm::max(rlen, 1e-24f);
    return -r * coeff;
}

glm::vec3 FluidConstraint::grad(int k, int j) {
    int i = m_particleIndices[k];
    Particle* p_i = m_allParticles->at(i);

    if (i == j) {
        glm::vec3 sum(0.0f);
        for (int n: m_neighbours[k]) {
            Particle* p_n = m_allParticles->at(n);
            sum += p_n->mass * spikyGrad(p_i->cpos - p_n->cpos);
        }
        return sum / m_density;
    }
    else {
        Particle* p_j = m_allParticles->at(j);
        return -p_j->mass * spikyGrad(p_i->cpos - p_j->cpos) / m_density;
    }
}

void FluidConstraint::AddParticle(int index) {
    m_particleIndices.push_back(index);
    m_allParticles->at(index)->num_constraints++;
    m_neighbours.emplace_back(); // empty vector
    m_lambdas.emplace(index, 0.0f);
    m_deltas.emplace_back(0.0f);
}
