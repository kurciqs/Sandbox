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
                auto* p = new Particle( RANDOM_POS_IN_BOUNDARIES, RANDOM_COLOR );
                m_particles.push_back(p);
            }

            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
            break;
        case ParticleSystemType::Pool:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( RANDOM_POS_IN_BOUNDARIES, RANDOM_COLOR );
                m_particles.push_back(p);
            }


            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }

            // the pool object
            AddObject(glm::vec3(0.0f, lowerBoundary.y + 2.0f, 0.0f), "Assets/Models/Test.obj");

            // this would be the particles, which represent a fluid
            for (int i = 0; i < 150; i++) {
                auto *p = new Particle(glm::vec3(rand() % 6 - rand() % 6, -rand() % 15 - 5, rand() % 6 - rand() % 6), glm::vec3(0.1f, 0.4f, 0.8f));
                p->radius = 0.5f;
                p->phase = Phase::Liquid;

                m_particles.push_back(p);
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }

        }
            break;
        case ParticleSystemType::Fluid:
        {
            m_constraints.emplace_back();
            m_constraints.emplace_back();
            m_constraints.emplace_back();

            std::vector<int> fluidParticles;
            for (int i = 0; i < numParticles; i++) {
                auto* p = new Particle( RANDOM_POS_IN_BOUNDARIES, glm::vec3(0.1f, 0.4f, 0.8f) );
                p->phase = Phase::Liquid;
                fluidParticles.push_back((int)m_particles.size());
                m_particles.push_back(p);
            }

            m_constraints[STANDARD].push_back( new FluidConstraint(m_particles, fluidParticles, 1.0f, 10.0f) );


            for (Particle* p: m_particles) {
                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
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

    for (RigidBody* rb: m_rigidBodies) {
        delete rb;
    }
    m_rigidBodies.clear();
}

void ParticleSystem::Destroy() {
    Clear();
}

void ParticleSystem::Update(float dt) {
    // (1)
    for (Particle* p : m_particles) {
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
                if (p1->rigidBodyID == -1 || p2->rigidBodyID == -1) {
                    m_constraints[CONTACT].push_back( new ContactConstraint(p1, p2, k_contact) );
                }
                else if (p1->rigidBodyID != p2->rigidBodyID) {
                    SDFData d1 = m_rigidBodies[p1->rigidBodyID]->GetSDFData(i);
                    SDFData d2 = m_rigidBodies[p2->rigidBodyID]->GetSDFData(j);
                    m_constraints[CONTACT].push_back( new RigidContactConstraint(p1, p2, d1, d2, k_contact) );
                }

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
    if (!Input::isKeyDown(GLFW_KEY_G)) {
        renderer.DrawParticles(m_particles);
    }

#ifndef NDEBUG
    bool drawp = Input::isKeyDown(GLFW_KEY_R);
    if (drawp) {
        for (int i = 0; i < m_particles.size(); i++) {
            Particle* p = m_particles[i];
            if (p->rigidBodyID != -1) {
                RigidBody* rb = m_rigidBodies[p->rigidBodyID];
                renderer.DrawLine(p->cpos + rb->GetSDFData(i).grad, p->cpos, glm::vec3(1.0f));
//                renderer.DrawLine(p->cpos + rb->GetSDFData(i).grad * glm::abs(rb->GetSDFData(i).mag), p->cpos, glm::vec3(0.0f, 0.2f, 0.4f));
            }
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

    if (!drawc) {
        for (Constraint *c: m_constraints[CONTACT]) {
            delete c;
        }
        m_constraints[CONTACT].clear();
    }
#endif
}

void ParticleSystem::AddParticle(glm::vec3 pos, glm::vec3 vel, glm::vec3 color, float r) {
    auto* p = new Particle(pos, color);
    p->vel = vel;
    p->radius = r;
    m_particles.push_back(p);
    m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, upperBoundary, lowerBoundary, 1.0f));
}

float map(float X, float A, float B, float C, float D) {
    return (X-A)/(B-A) * (D-C) + C;
}

void ParticleSystem::AddCube(glm::vec3 pos, glm::vec3 vel, int width, int height, int depth, glm::vec3 color) {
    auto* rb = new RigidBody((int)m_rigidBodies.size());

    glm::vec3 boxSize((float)width / 2.0f + 0.5f,  (float)height / 2.0f + 0.5f, (float)depth / 2.0f + 0.5f);
    float step = 1.0f;
    for (float i = -boxSize.x + 1; i <= boxSize.x - 1; i += step) {
        for (float j = -boxSize.y + 1; j <= boxSize.y - 1; j += step) {
            for (float k = -boxSize.z + 1; k <= boxSize.z - 1; k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + pos;

                // SDF Calc:
                SDFData d = Generator::SDFCube(ppos - pos, boxSize, step);
                if (d.mag >= 0.0f) {
                    continue;
                }

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->radius = step / 2.0f;
                p->vel = vel;

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

void ParticleSystem::AddBall(glm::vec3 center, glm::vec3 vel, float radius, glm::vec3 color) {
    auto* rb = new RigidBody((int)m_rigidBodies.size());
    float step = 1.0f;

    for (float i = -glm::floor(radius); i < glm::ceil(radius); i += step) {
        for (float j = -glm::floor(radius); j < glm::ceil(radius); j += step) {
            for (float k = -glm::floor(radius); k < glm::ceil(radius); k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + center;

                SDFData d = Generator::SDFBall(ppos - center, radius, step);
                if (d.mag >= 0.0f) {
                    continue;
                }

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->radius = step / 2.0f;
                p->vel = vel;

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

void ParticleSystem::AddObject(glm::vec3 offset, const std::string& fileToObj, glm::vec3 color, glm::vec3 vel) {
    std::vector<RigidBody*> rbs = Generator::RigidBodiesFromOBJ(fileToObj, (int) m_rigidBodies.size(), (int) m_particles.size(), offset, color, vel);
    for (RigidBody* rb : rbs) {
        for (Particle *p: rb->particles) {
            m_particles.push_back(p);
            m_constraints[STANDARD].push_back(new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f));
        }
        m_rigidBodies.push_back(rb);
        m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
    }
}

void ParticleSystem::AddTorus(glm::vec3 center, glm::vec3 vel, float innerRadius, float outerRadius, glm::vec3 color) {
    auto* rb = new RigidBody((int)m_rigidBodies.size());
    float step = 1.0f;

    for (float i = -glm::floor(outerRadius) - 2.0f; i < glm::ceil(outerRadius) + 2.0f; i += step) {
        for (float j = -glm::floor(outerRadius / 2.0f) - 2.0f; j < glm::ceil(outerRadius / 2.0f) + 2.0f; j += step) {
            for (float k = -glm::floor(outerRadius) - 2.0f; k < glm::ceil(outerRadius) + 2.0f; k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + center;

                SDFData d = Generator::SDFTorus(ppos - center, glm::vec2(outerRadius, innerRadius), step);
                if (d.mag >= 0.0f) {
                    continue;
                }

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->radius = step / 2.0f;
                p->vel = vel;

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

void ParticleSystem::AddCylinder(glm::vec3 center, glm::vec3 vel, float height, float radius, glm::vec3 color) {
    auto* rb = new RigidBody((int)m_rigidBodies.size());
    float step = 1.0f;

    for (float i = -glm::floor(radius) - 1.0f; i < glm::ceil(radius) + 1.0f; i += step) {
        for (float j = -glm::floor(height / 2.0f) - 1.0f; j < glm::ceil(height / 2.0f) + 1.0f; j += step) {
            for (float k = -glm::floor(radius) - 1.0f; k < glm::ceil(radius) + 1.0f; k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + center;

                SDFData d = Generator::SDFCylinder(ppos - center, radius, height, step);
                if (d.mag >= 0.0f) {
                    continue;
                }

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->radius = step / 2.0f;
                p->vel = vel;

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}

void ParticleSystem::AddCone(glm::vec3 center, glm::vec3 vel, float angle /*radians*/, float height, glm::vec3 color) {
    auto* rb = new RigidBody((int)m_rigidBodies.size());
    float step = 1.0f;

    float cosangle = cosf(angle);
    float sinangle = sinf(angle);
    float radius = (height / cosangle) * sinangle;

    for (float i = -glm::floor(radius) - 1.0f; i < glm::ceil(radius) + 1.0f; i += step) {
        for (float j = -glm::floor(height) - 1.0f; j < 0.0f; j += step) {
            for (float k = -glm::floor(radius) - 1.0f; k < glm::ceil(radius) + 1.0f; k += step) {
                glm::vec3 ppos = glm::vec3(i, j, k) + center;

                SDFData d = Generator::SDFCone(ppos - center, glm::vec2(sinangle, cosangle), height, step);
                if (d.mag >= 0.0f) {
                    continue;
                }

                auto *p = new Particle(ppos, color);
                p->rigidBodyID = rb->ID;
                p->radius = step / 2.0f;
                p->vel = vel;

                rb->AddVertex(p, d, (int)m_particles.size()); // before pushing to main array
                m_particles.push_back(p);

                m_constraints[STANDARD].push_back( new BoxBoundaryConstraint(p, lowerBoundary, upperBoundary, 1.0f) );
            }
        }
    }
    rb->CalculateOffsets();
    m_rigidBodies.push_back(rb);
    m_constraints[SHAPE].push_back( new RigidShapeConstraint(rb, k_shape) );
}