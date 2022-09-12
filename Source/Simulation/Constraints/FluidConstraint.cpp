#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;

    m_particles = allParticles;
    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        m_particles[i]->num_constraints++;
        m_neighbours.emplace_back(); // empty vector
        m_lambdas.push_back(0.0f);
    }
}

FluidConstraint::~FluidConstraint() {
    for (int i : m_particleIndices) {
        m_particles[i]->num_constraints--;
    }
}

void FluidConstraint::Project() {
    // NOTE: m_particles contains all of them
    for (int localInd = 0; localInd < m_particleIndices.size(); localInd++) {
        m_neighbours[localInd].clear();
        int i = m_particleIndices[localInd];  // fluid particle index (global)
        Particle *p_i = m_particles[i];

        float density_i = 0.0f; // roh_i
        float denom_i = 0.0f;

        for (int j = 0; j < m_particles.size(); j++) { // loop through all, find neighbours

            if (i != j) {
                Particle *p_j = m_particles[j];

                glm::vec3 diff = p_i->cpos - p_j->cpos;
                float d2 = glm::dot(diff, diff);

                if (d2 < H2 /* otherwise division by zero determines neighbour */ ) {
                    // found neighbour
                    float incr = poly6(d2) * p_j->mass;

                    if (p_j->phase == Phase::Solid) {
                        incr *= S_SOLID;
                    }

                    density_i += incr;
                    m_neighbours[localInd].push_back(j);

                    // TODO;
                    glm::vec3 gr = grad(localInd, j);
                    float gradSq = glm::dot(gr, gr);
                    denom_i += gradSq;
                }
            }

            else {
                m_neighbours[localInd].push_back(j);
                density_i += poly6(0.0f) * p_i->mass;
            }
        }


        m_lambdas[localInd] = -((density_i / m_density) - 1.0f) / (denom_i + EPSILON_RELAX);
    }

    for (int localInd = 0; localInd < m_particleIndices.size(); localInd++) {
        int i = m_particleIndices[localInd];  // fluid particle index (global)

        Particle *p_i = m_particles[i];
        float lambda_i = m_lambdas[localInd];
        glm::vec3 corr(0.0f);

        for (int localInd2 = 0; localInd2 < m_neighbours[localInd].size(); localInd2++) {
            int j = m_neighbours[localInd][localInd2];  // index to localInd's neighbour number localInd2 (global)

            if (i == j) continue;

            Particle *p_j = m_particles[j];
            float lambda_j = m_lambdas[j];

            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float len = glm::length(diff);
            float scorr = -K_CORR * powf(poly6(len * len) / poly6(MAG_Q_CORR * MAG_Q_CORR * H * H), N_CORR);

            corr += (lambda_i + lambda_j + scorr) * spikyGrad(diff, len);
        }
        if (!p_i->fixed) p_i->cpos += corr / m_density * m_stiffness / ((float)p_i->num_constraints + (float)m_neighbours[localInd].size());
    }
}

void FluidConstraint::Draw(Renderer &renderer) {

}

float FluidConstraint::poly6(float r2) {
    if(r2 >= H2) return 0.0f;
    float term2 = (H2 - r2);
    return (315.0f / (64.0f * (float)M_PI * H9)) * (term2 * term2 * term2);
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r, float r2) {
    if(r2 >= H) return {};
    if(r2 == 0) return {};
    return -glm::normalize(r) * (45.0f / ((float)M_PI * H6)) * (H - r2) * (H - r2);
}

glm::vec3 FluidConstraint::grad(int k, int j) {
    int i = m_particleIndices[k];
    Particle* p_i = m_particles[i];
    Particle* p_j = m_particles[j];

    glm::vec3 diff = p_i->cpos - p_j->cpos;
    float len = glm::length(diff);

    if (p_i != p_j) {
        return -spikyGrad(diff, len) / m_density;
    }

    glm::vec3 res(0.0f);
    for (int n : m_neighbours[k]) {
        p_j = m_particles[m_neighbours[k][n]];
        diff = p_i->cpos - p_j->cpos;
        len = glm::length(diff);
        res += (p_j->phase == Phase::Solid ? S_SOLID : 1.f) * spikyGrad(diff, len);

    }

    return res / m_density;
}
