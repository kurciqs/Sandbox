#version 460 core

layout (location = 0) in vec3 aPos; // from the quad

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform float radius;
uniform vec3 ucolor;

out vec2 vPosOnQuad;
out vec3 color;

void main() {
    vPosOnQuad = (aPos.xy) * vec2(radius);
    color = ucolor;
    iradius = radius;
    gl_Position = proj * view * (model * (vec4(aPos * radius, 1.0)));
}
