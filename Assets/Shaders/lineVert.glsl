#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 outColor;

uniform mat4 model;
uniform mat4 cam;

void main() {
    outColor = aColor;
    gl_Position = cam * (model * vec4(aPos, 1.0));
}