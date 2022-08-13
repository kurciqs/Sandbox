#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(int numParticles, ParticleSystemType type)
{
    switch (type) {
        case ParticleSystemType::Testing:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( glm::vec3(rand() % 10 - rand() % 10, 5, rand() % 10 - rand() % 10), RANDOM_COLOR );
                m_particles.push_back(p);
            }

            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }

            AddCube(glm::vec3(5.0f), glm::vec3(0.0f), 4, 4, 4, RANDOM_COLOR);
            AddBall(glm::vec3(0.0f), glm::vec3(0.0f), 3.0f, RANDOM_COLOR);
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
#ifdef GRAVITY
        if (!p->fixed) p->ApplyForce(m_globalForce * p->mass);
#endif
        p->vel += dt * (p->invMass * p->force);
        p->vel *= 0.98f;
        p->cpos = p->pos + (dt * p->vel);
        // TODO: (4) mass scaling (only for stiff stacks)
    }
    // (5)


    // (6)
    // TODO
    for (int i = 0; i < m_particles.size(); i++) {
        Particle* p1 = m_particles[i];

        // (7) dumb, I know
        for (int j = i + 1; j < m_particles.size(); j++) {
            Particle* p2 = m_particles[j];
            float dist = glm::fastDistance(p1->cpos, p2->cpos);
            if (dist < p1->radius + p2->radius) {
                // (8)
                if (p1->rigidBodyID == 0 && p2->rigidBodyID == 0) {
                    // normal collision between two normal particles
                    m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                }
                else if (p1->rigidBodyID == 0 || p2->rigidBodyID == 0) {
                    // collision between normal and rigidbody
                    m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                }
                else if (p1->rigidBodyID != p2->rigidBodyID) {
                    // collision between two rigid bodies, that aren't the same (their IDs aren't equal) AND they are not 0
                    m_constraints[CONTACT].push_back( new RigidContactConstraint(m_rigidBodies[p1->rigidBodyID - 1], m_rigidBodies[p2->rigidBodyID - 1], p1, p2, k_contact) );
                }
                // else do nothing p1->rigidBodyID == p2->rigidBodyID (no collision required in a rigidbody)
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
    if (drawp) {
        for (Particle* p : m_particles) {
            renderer.DrawLine(p->pos, glm::vec3(0.0f), glm::vec3(1.0f));
        }
    }

    bool drawc = Input::isKeyDown(GLFW_KEY_C);
    if (drawc) {
        for (const ConstraintGroup &g: m_constraints) {
            for (Constraint *c: g) {
                c->Draw(renderer);
            }
        }
    }

    for (Constraint* c: m_constraints[CONTACT]) {
        delete c;
    }
    m_constraints[CONTACT].clear();
#endif
}

void ParticleSystem::AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r) {
    auto* p = new Particle(pos, color);
    p->vel = vel;
    p->radius = r;
    m_particles.push_back(p);
    m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, upperBoundary, lowerBoundary, 0.45f));
}

void ParticleSystem::AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color) {
    int lastIndex = 0;
    for (int i = -width / 2; i < width / 2; i++) {
        for (int j = -height / 2; j < height / 2; j++) {
            for (int k = -depth / 2; k < depth / 2; k++) {
                auto *p = new Particle(glm::vec3(i, j, k) + pos, color);
                p->rigidBodyID = (int)m_rigidBodies.size() + 1;
                p->vel = vel;
                m_particles.push_back(p);

                lastIndex++;
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    m_rigidBodies.push_back( new RigidBody((int)m_rigidBodies.size() + 1, (int)m_particles.size() - lastIndex, (int)m_particles.size() - 1, m_particles) );
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(m_rigidBodies[m_rigidBodies.size() - 1], m_particles, k_shape) );
}

void ParticleSystem::AddBall(glm::vec3 pos, glm::vec3 vel, float radius, glm::vec3 color) {
    int lastIndex = 0;
    for (int i = -(int)glm::round(radius) - 1; i < (int)glm::round(radius) + 1; i++) {
        for (int j = -(int)glm::round(radius) - 1; j < (int)glm::round(radius) + 1; j++) {
            for (int k = -(int)glm::round(radius) - 1; k < (int)glm::round(radius) + 1; k++) {
                if (glm::length2(glm::vec3(i, j, k)) + 2 < radius * radius) {
                    auto *p = new Particle(glm::vec3(i, j, k) + pos, color);
                    p->rigidBodyID = (int) m_rigidBodies.size() + 1;
                    p->vel = vel;
                    m_particles.push_back(p);

                    lastIndex++;
                    m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
                }
            }
        }
    }
    m_rigidBodies.push_back( new RigidBody((int)m_rigidBodies.size() + 1, (int)m_particles.size() - lastIndex, (int)m_particles.size() - 1, m_particles) );
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(m_rigidBodies[m_rigidBodies.size() - 1], m_particles, k_shape) );
}