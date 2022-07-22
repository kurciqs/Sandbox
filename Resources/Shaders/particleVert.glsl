#version 460 core

// TODO: okay so GEPE3D uses radius for the width of the base quad, but I don't -> need to scale the base quad

layout (location = 0) in vec3 aPos; // from the quad
layout (location = 1) in vec3 aOffset;
layout (location = 2) in vec3 aColor;
layout (location = 3) in float aRadius;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = proj * view * (model * vec4(aPos + aOffset, 1.0)) * aRadius;
}