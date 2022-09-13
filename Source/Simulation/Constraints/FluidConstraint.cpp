#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, float k, float density) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;

    m_particles = allParticles;
    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        m_particles[i]->num_constraints++;
        m_neighbours.emplace_back(); // empty vector
        m_lambdas.emplace(i, 0.0f);
        m_deltas.emplace_back(0.0f);
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

                if (p_j->fixed) continue;

                glm::vec3 diff = p_i->cpos - p_j->cpos;
                float d2 = glm::dot(diff, diff);

                if (d2 < H2) {
                    // found neighbour
                    m_neighbours[localInd].push_back(j);

                    float incr = poly6(glm::fastSqrt(d2)) * p_j->mass;

                    if (p_j->phase == Phase::Solid) {
                        incr *= S_SOLID;
                    }

                    density_i += incr;

                    glm::vec3 gr = grad(localInd, j);
                    denom_i += glm::dot(gr, gr);
                }
            }
            else {
                m_neighbours[localInd].push_back(j);
                density_i += poly6(0.0f) * p_i->mass;
            }
        }

        glm::vec3 gr = grad(localInd, i);
        denom_i += glm::dot(gr, gr);

        m_lambdas.insert_or_assign(i, -((density_i / m_density) - 1.0f) / (denom_i + EPSILON_RELAX));
    }

    for (int localInd = 0; localInd < m_particleIndices.size(); localInd++) {
        glm::vec3 corr(0.0f);
        int i = m_particleIndices[localInd];  // fluid particle index (global)

        float lambda_i = m_lambdas[i];
        Particle *p_i = m_particles[i];

        for (int localInd2 = 0; localInd2 < m_neighbours[localInd].size(); localInd2++) {
            int j = m_neighbours[localInd][localInd2];  // index to localInd's neighbour number localInd2 (global)

            if (i == j) continue;

            Particle *p_j = m_particles[j];
            glm::vec3 diff = p_i->cpos - p_j->cpos;

            float lambda_j = m_lambdas[j];
            float len = glm::length(diff);
            float scorr = -K_CORR * powf(poly6(len) / poly6(MAG_Q_CORR * H), N_CORR);

            glm::vec3 incr = (lambda_i + lambda_j + scorr) * spikyGrad(diff, len);
            corr += incr;
        }

        m_deltas[localInd] = corr / m_density;
    }

    for (int localInd = 0; localInd < m_particleIndices.size(); localInd++) {
        int i = m_particleIndices[localInd];
        Particle* p_i = m_particles[i];
        if (!p_i->fixed) p_i->cpos += m_deltas[localInd] / ((float) p_i->num_constraints + (float) m_neighbours[localInd].size());
    }
}

void FluidConstraint::Draw(Renderer &renderer) {

}

// TODO: write thsi more coherently
float FluidConstraint::poly6(float dist) {
    dist = glm::clamp(dist, 0.0f, H);
    float term2 = (H * H - dist * dist);
    return (315.0f / (64.0f * (float)M_PI * H9)) * (term2 * term2 * term2);
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r, float dist) {
    dist = glm::clamp(dist, 0.0f, H);
    if (dist == 0.0f) {
        return glm::vec3(0.0f);
    }
    return -glm::normalize(r) * (45.0f / ((float)M_PI * H6)) * (H - dist) * (H - dist);
}

glm::vec3 FluidConstraint::grad(int k, int j) {
    int i = m_particleIndices[k];
    Particle* p_i = m_particles[i];
    Particle* p_j = m_particles[j];

    glm::vec3 diff = p_i->cpos - p_j->cpos;
    float len = glm::length(diff);

    glm::vec3 res(0.0f);

    if (p_i != p_j) {
        res = -spikyGrad(diff, len);
    }
    else {
        for (int n: m_neighbours[k]) {
            p_j = m_particles[n];
            diff = p_i->cpos - p_j->cpos;
            len = glm::length(diff);
            glm::vec3 sg = spikyGrad(diff, len);
            res += (p_j->phase == Phase::Solid ? S_SOLID : 1.f) * sg;
        }
    }
    return res / m_density;
}
