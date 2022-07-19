#ifndef SANDBOX_PARTICLESYSTEM_H
#define SANDBOX_PARTICLESYSTEM_H

#include "Particle.h"


class ParticleSystem {
public:
    void Update(float dt);
    void Render();
    void Shutdown();

};


#endif //SANDBOX_PARTICLESYSTEM_H
