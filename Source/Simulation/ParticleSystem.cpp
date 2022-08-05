#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type) {
    switch (type) {
        case ParticleSystemType::Testing:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            for (int i = 0; i < numParticles; i++) {
                float radius = (rand() % 20) / 20.0f;
                auto* p = new Particle( glm::vec3(rand() % 5, rand() % 5, rand() % 5), RANDOM_COLOR, radius /2.0f, radius );

                m_particles.push_back(p);

                m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 0.45f));
            }
            m_constraints[STANDARD].push_back(new PositionConstraint(m_particles[0], glm::vec3(0.0f), 0.02f));
        }
            break;
        default:
            break;
    }
}

void ParticleSystem::Clear() {
    for (auto& g: m_constraints) {
        for (auto *c: g) {
            delete c;
        }
        g.clear();
    }
    m_constraints.clear();

    // DELETE CONSTRAINTS FIRST, OTHERWISE SEGFAULTS, IDIOT
    for (auto* p : m_particles) {
        delete p;
    }
    m_particles.clear();
}

void ParticleSystem::Destroy() {
    Clear();
}

void ParticleSystem::Update(float dt) {
    // (1)
    for (Particle* p : m_particles) {
        if (Input::isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT) && !p->fixed)
            p->ApplyForce(glm::vec3( (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100), (rand() % 100 - rand() % 100) ));

        if (!p->fixed) p->ApplyForce(m_globalForce * p->mass);

        p->vel += dt * (p->invMass * p->force);
        p->vel *= 0.975f;
        p->cpos = p->pos + (dt * p->vel);
        // TODO: (4) mass scaling (only for stiff stacks)
    }
    // (5)


    // (6)
    // TODO
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p1 = m_particles[i];

        // (7) dumb, I know
        for (int j = 0; j < m_particles.size(); j++) {
            if (i == j)
                continue;

            Particle* p2 = m_particles[j];
            float dist = glm::distance(p1->cpos, p2->cpos);
            if (dist < p1->radius + p2->radius) {
                // TODo: push a contact constraint here depending on the phase
                // (8)
                m_constraints[CONTACT].push_back(new ContactConstraint(p1, p2, k_contact));
            }
        }
    }
    // (9)


    // (10)
    // TODO: stabilization
    // (15)


    // (16)
    for (int i = 0; i < SOLVER_ITERATIONS; i++) {
        for (const ConstraintGroup& g : m_constraints) {
            for (Constraint* c : g) {
                c->Project(); // NOTE: also the counts are implemented careful
            }
        }
    }
    // (17)


    // (22)
    for (Particle* p : m_particles) {
        p->force = glm::vec3(0.0f);
        p->vel = (p->cpos - p->pos) / dt;
        // TODO: (25, 26)

        // sleeping:
        if (glm::fastDistance(p->cpos, p->pos) < PARTICLE_SLEEPING_EPSILON)
        {
            p->vel = glm::vec3(0.0f);
            continue;
        }
        p->pos = p->cpos;
    }
    // (28)

#ifdef NDEBUG
    for (Constraint* c: m_constraints[CONTACT]) {
        delete c;
    }
    m_constraints[CONTACT].clear();
#endif
}

void ParticleSystem::Draw(Renderer& renderer) {
    renderer.DrawParticles(m_particles);

#ifndef NDEBUG
    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    for (Particle* p : m_particles) {
        if (drawp) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
    }

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    for (const ConstraintGroup& g : m_constraints) {
        for (Constraint* c : g) {
            if (drawc) {
                c->Draw(renderer);
            }
        }
    }

    // not cool:
    for (Constraint* c: m_constraints[CONTACT]) {
        delete c;
    }
    m_constraints[CONTACT].clear();
#endif
}

void ParticleSystem::AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color) {
    auto* p = new Particle(pos, color);
    p->vel = vel;
    m_particles.push_back(p);
    m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, upperBoundary, lowerBoundary, 0.45f));
}


