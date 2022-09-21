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
    for (int k = 0; k < m_particleIndices.size(); k++) {
        m_neighbours[k].clear();
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        for (int j = 0; j < m_allParticles->size(); j++) {
            Particle *p_j = m_allParticles->at(j);
            glm::vec3 diff = p_i->cpos - p_j->cpos;
            float d2 = glm::dot(diff, diff);
            if (d2 < H2) {
                m_neighbours[k].push_back(j);
            }
        }
    }

    m_lambdas.clear();
    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        float density_i = 0.0f;
        float denom_i = 0.0f;
        for (int j: m_neighbours[k]) {
            Particle *p_j = m_allParticles->at(j);

            glm::vec3 r = p_i->cpos - p_j->cpos;
            density_i += (p_j->phase == Phase::Solid ? S_SOLID : 1.0f) * p_j->mass * poly6(r);

            glm::vec3 gr = grad(k, j);
            denom_i += glm::dot(gr, gr) * p_j->invMass;
        }


        m_lambdas.insert_or_assign(i, -((density_i / m_density) - 1.0f) / (denom_i + EPSILON_RELAX));
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        float lambda_i = m_lambdas[i];

        const float corrW = poly6(glm::vec3(MAG_Q_CORR, 0.0f, 0.0f) * H);
        glm::vec3 corr(0.0f);
        for (int j: m_neighbours[k]) {
            Particle *p_j = m_allParticles->at(j);
            if (p_j->phase == Solid) {
                continue;
            }
            float lambda_j = m_lambdas[j];

            glm::vec3 diff = p_i->cpos - p_j->cpos;

            float scorr = -K_CORR * glm::pow(poly6(diff) / corrW, N_CORR);
            corr += p_j->mass * (lambda_i + lambda_j + scorr) * spikyGrad(diff);
        }

        m_deltas[k] = p_i->invMass * corr / m_density;
    }

    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        glm::vec3 delta = m_stiffness * m_deltas[k];
        if (!p_i->fixed) p_i->cpos += delta / (float) m_neighbours[k].size();
    }

    // viscosity / vorticity

//    std::unordered_map<int, float> densities;
    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);

        glm::vec3 omega(0.0f);
        glm::vec3 vnew(0.0f);
        for (int j: m_neighbours[k]) {
            Particle *p_j = m_allParticles->at(j);

//            vnew += (p_i->mass / densities[j]) * poly6(p_i->cpos - p_j->cpos) * (p_j->vel - p_i->vel);
            vnew += poly6(p_i->cpos - p_j->cpos) * (p_j->vel - p_i->vel);
            omega += glm::cross((p_j->vel - p_i->vel), spikyGrad((p_i->cpos - p_j->cpos)));
        }

        p_i->viscosity = vnew * m_viscosityMag;

        float omegaLength = glm::fastLength(omega);
        if (omegaLength == 0.0f) {
            return;
        }

        glm::vec3 eta(0.0f);
        for (int j: m_neighbours[k]) {
            Particle *p_j = m_allParticles->at(j);
            if (p_j->phase == Solid) {
                continue;
            }
            glm::vec3 diff = p_i->cpos - p_j->cpos;
            eta += spikyGrad(diff) * omegaLength;
        }

        if (eta.x == 0.0f && eta.y == 0.0f && eta.z == 0.0f) {
            return;
        }
        glm::vec3 N = glm::fastNormalize(eta);
        p_i->ApplyForce(VORTICITY_COEF * (glm::cross(N, eta)));
    }
}

void FluidConstraint::Draw(Renderer &renderer) {
    for (int k = 0; k < m_particleIndices.size(); k++) {
        int i = m_particleIndices[k];
        Particle *p_i = m_allParticles->at(i);
        glm::vec3 delta = m_stiffness * m_deltas[k] / (float) m_neighbours[k].size();
        renderer.DrawLine(p_i->cpos, p_i->cpos + delta, glm::vec3(1.0f));
    }
}

float FluidConstraint::poly6(const glm::vec3& r) {
    float rlen = glm::fastLength(r);
    if (rlen > H || rlen == 0.0f) {
        return 0.0f;
    }

    float term2 = (H2 - rlen * rlen);
    return k_Poly6 * (term2 * term2 * term2);
}

glm::vec3 FluidConstraint::spikyGrad(const glm::vec3 &r) {
    float rlen = glm::fastLength(r);
    if (rlen > H) {
        return glm::vec3(0.0f);
    }
    float coeff = (H - rlen) * (H - rlen) * k_SpikyGrad;
    coeff /= glm::max(rlen, 0.001f);
    return -r * coeff;
}

glm::vec3 FluidConstraint::grad(int k, int j) {
    int i = m_particleIndices[k];
    Particle* p_i = m_allParticles->at(i);

    if (i == j) {
        glm::vec3 sum(0.0f);
        for (int n: m_neighbours[k]) {
            Particle* p_n = m_allParticles->at(n);
            sum += (p_n->phase == Phase::Solid ? S_SOLID : 1.0f) * p_n->mass * spikyGrad(p_i->cpos - p_n->cpos);
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
