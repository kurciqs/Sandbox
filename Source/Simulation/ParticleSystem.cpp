#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    for (int i = 0; i < numParticles; i++) {
        auto* p = new Particle{.pos = glm::vec3{rand() % 10, rand() % 10, rand() % 10}};
        m_particles.push_back(p);
    }
    // TODO: Constraints
}

void ParticleSystem::Clear() {
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();
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
