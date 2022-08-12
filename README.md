# Sandbox
 3D Sandbox Game using Unified Particle Physics for Real-Time Applications

## Build Instructions

    git clone https://github.com/kurciqs/Sandbox.git
    cd Sandbox
    # There must be two directories, otherwise the working directory won't be correct:
    mkdir Binaries/Build -r
    cd Binaries/Build
    cmake ../.. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE
    make
    ./Sandbox

## Debugging

    git clone https://github.com/kurciqs/Sandbox.git
    cd Sandbox
    # There must be two directories, otherwise the working directory won't be correct:
    mkdir Binaries/Debug -r
    cd Binaries/Debug
    cmake ../.. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG
    make
    ./Sandbox