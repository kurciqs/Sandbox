#version 460 core

// TODO: okay so GEPE3D uses -radius for the size of the base quad, but I don't -> need to scale the base quad

layout (location = 0) in vec3 aPos; // from the quad

uniform mat4 model;
uniform mat4 cam;

void main() {
    gl_Position = cam * (model * vec4(aPos, 1.0));
}