# Sandbox
 3D Sandbox Game using Unified Particle Physics for Real-Time Applications

### Build Instructions

    git clone https://github.com/kurciqs/Sandbox.git
    cd Sandbox
    # There must be two directories, otherwise the working directory won't be correct:
    mkdir Binaries/Build -r
    cd Binaries/Build
    cmake ../.. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=RELEASE
    make
    ./Sandbox

    

There is also a precompiled zip for windows: https://kurciqs1.itch.io/sandbox

### Visuals

![Stacked Donuts](Assets/Images/Donuts.png "Stacked Donuts")
![Fluid](Assets/Images/Fluid.png "Fluid")
![UI_Showcase](Assets/Images/UI_Showcase.png "UI_Showcase")
![Cloth](Assets/Images/Cloth.png "Cloth")

### References
*  M. Macklin, M. Müller, N. Chentanez and T.Y. Kim, "Unified particle physics for real-time applications", ACM Trans. Graph. 33, 4, 2014
*  M. Macklin and M. Müller, "Position based fluids", ACM Trans. Graph. 32, 4, 2013
*  M. Müller, B. Heidelberger, M. Hennix and J. Ratcliff, "Position based dynamics", Journal of Visual Communication and Image Representation 18, 2, 2007
 
