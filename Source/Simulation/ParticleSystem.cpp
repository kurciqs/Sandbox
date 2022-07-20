#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    for (int i = 0; i < numParticles; i++) {
        m_particles.push_back(new Particle{.pos = glm::vec3{rand() % 10, rand() % 10, rand() % 10}});
    }
    // NOTE: for now 10 empty constraints and 1 constraint group
    ConstraintGroup g;
    for (int i = 0; i < 10; i++) {
        Constraint* c = new DistanceConstraint(m_particles[0], m_particles[1], 1.0f, 1.0f);
        g.push_back(c);
    }
    m_constraints.push_back(g);
}

void ParticleSystem::Clear() {
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();

    for (auto& g: m_constraints) {
        for (auto *c: g) {
            delete c;
        }
        g.clear();
    }
    m_constraints.clear();
}

void ParticleSystem::Destroy() {
    Clear();
}

void ParticleSystem::Update(float dt) {
    // TODO: .
}

void ParticleSystem::Draw() {
    // TODO: .
}
