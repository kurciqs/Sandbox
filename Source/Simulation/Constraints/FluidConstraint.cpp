#include "FluidConstraint.h"

FluidConstraint::FluidConstraint(int ID, std::vector<Particle*>& allParticles, const std::vector<int>& fluidPartilces, glm::vec3 color, float k, float density, float viscosityMag) {
    m_stiffness = 1.0f - powf((1.0f - k), 1.0f / SOLVER_ITERATIONS);
    m_density = density;
    m_viscosityMag = viscosityMag;
    m_ID = ID;

    m_color = color;
    m_allParticles = &allParticles;
    for (int i : fluidPartilces) {
        m_particleIndices.push_back(i);
        m_allParticles->at(i)->num_constraints++;
        m_allParticles->at(i)->color = m_color;
        m_allParticles->at(i)->fluidID = m_ID;
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
    std::unordered_map<int, float> densities;
    for (int i = 0; i < m_allParticles->size(); i++) {
        float density = 0.0f;
        Particle *p_i = m_allParticles->at(i);
        for (Particle *p_j: *m_allParticles) {
            density += p_j->mass * W(p_i->cpos - p_j->cpos);
        }
        densities.insert({i, density});

        printf("%f\n", density);
        p_i->color = glm::vec3(density) / 40.0f;
    }

    for (int i: m_particleIndices) {
        Particle *p_i = m_allParticles->at(i);

//        glm::vec3 viscosity = p_i->mass
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

float FluidConstraint::W(const glm::vec3& r) {
    float sigma = 8.0f / (glm::pi<float>() * glm::pow(m_kernelRadius, 3.0f)); // normalization constant
    float q = (1.0f / m_kernelRadius) * glm::length(r);
    if (0.0f <= q && q <= 0.5f) {
        float q2 = q*q;
        float q3 = q2*q;
        return sigma * (6.0f * (q3 - q2) + 1.0f);
    }
    else if (0.5f < q && q <= 1.0f) {
        return sigma * 2.0f * glm::pow(1.0f - q, 3.0f);
    }
    else {
        return 0.0f;
    }
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
    m_allParticles->at(index)->num_constraints++; // TODO
    m_neighbours.emplace_back(); // empty vector
    m_lambdas.emplace(index, 0.0f);
    m_deltas.emplace_back(0.0f);
}
