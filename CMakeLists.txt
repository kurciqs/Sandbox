cmake_minimum_required(VERSION 3.2)
project(Sandbox)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES Source/main.cpp Source/Window/Window.cpp Source/Window/Window.h Source/Utils/Utils.cpp Source/Utils/Utils.h Source/Window/Input.cpp Source/Window/Input.h
        Source/Graphics/Camera.cpp Source/Graphics/Camera.h Source/Graphics/OpenGL/Objects.cpp Source/Graphics/OpenGL/Objects.h
        Source/Graphics/OpenGL/Shader.cpp Source/Graphics/OpenGL/Shader.h
        Source/Graphics/Renderer.cpp Source/Graphics/Renderer.h Source/Simulation/Particle.cpp Source/Simulation/Particle.h
        Source/Simulation/ParticleSystem.cpp Source/Simulation/ParticleSystem.h
        Source/Simulation/Constraint.h Source/Simulation/Constraints/DistanceConstraint.cpp Source/Simulation/Constraints/DistanceConstraint.h
        Source/Simulation/Constraints/ContactConstraint.cpp Source/Simulation/Constraints/ContactConstraint.h
        Source/Simulation/Constraints/BoxBoundaryConstraint.cpp Source/Simulation/Constraints/BoxBoundaryConstraint.h
        Source/Simulation/RigidBody.cpp Source/Simulation/RigidBody.h
        Source/Simulation/Constraints/PositionConstraint.cpp Source/Simulation/Constraints/PositionConstraint.h
        Source/Simulation/Constraints/RigidShapeConstraint.cpp Source/Simulation/Constraints/RigidShapeConstraint.h Source/Simulation/Constraints/RigidContactConstraint.cpp
        Source/Simulation/Constraints/RigidContactConstraint.h Source/Utils/SDFGenerator.cpp Source/Utils/SDFGenerator.h Source/Utils/tmd.h)

include_directories(
        ${PROJECT_SOURCE_DIR}/Source
        ${PROJECT_SOURCE_DIR}/Libraries/glad/include
        ${PROJECT_SOURCE_DIR}/Libraries/glfw/include
        ${PROJECT_SOURCE_DIR}/Libraries/glm/include
        ${PROJECT_SOURCE_DIR}/Libraries/imgui/include
        ${PROJECT_SOURCE_DIR}/Libraries/json/include
        ${PROJECT_SOURCE_DIR}/Libraries/stb/include
        ${PROJECT_SOURCE_DIR}/Libraries/tinyobjloader/include
        ${PROJECT_SOURCE_DIR}/Libraries/tinyobjloader/
)

IF(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdelete-non-virtual-dtor -Werror")
ENDIF()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") # GCC / MinGW
    target_link_libraries(${PROJECT_NAME} -static-libgcc -static-libstdc++)
endif()

IF (WIN32)
    target_link_libraries(${PROJECT_NAME} -static winpthread)
ENDIF()

set(GLFW_BUILD_EXAMPLES false)
set(GLFW_BUILD_TESTS false)
add_subdirectory(Libraries/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(Libraries/glad)
target_link_libraries(${PROJECT_NAME} glad)

add_subdirectory(Libraries/glm)
target_link_libraries(${PROJECT_NAME} glm)

add_subdirectory(Libraries/stb)
target_link_libraries(${PROJECT_NAME} stb)

add_subdirectory(Libraries/tinyobjloader)
target_link_libraries(${PROJECT_NAME} tinyobjloader)

#add_subdirectory(Libraries/imgui)
#target_link_libraries(${PROJECT_NAME} imgui)