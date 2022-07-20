#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    for (int i = 0; i < numParticles; i++) {
        m_particles.push_back(new Particle{.pos = glm::vec3{rand() % 10, rand() % 10, rand() % 10}});
    }
    // NOTE: for now 100 empty constraints and 1 constraint group
    m_constraints.reserve(1);
    for (int i = 0; i < 100; i++) {
        m_constraints[0].push_back(new DistanceConstraint(m_particles[0], m_particles[1], 1.0f, 1.0f));
    }
}

void ParticleSystem::Clear() {
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();

    for (ConstraintGroup& group : m_constraints) {
        for (auto *c: group) {
            delete c;
        }
        group.clear();
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
