#version 460 core

// TODO: okay so GEPE3D uses radius for the width of the base quad, but I don't -> need to scale the base quad

layout (location = 0) in vec3 aPos; // from the quad

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

void main() {
    gl_Position = proj * view * (model * vec4(aPos, 1.0));
}