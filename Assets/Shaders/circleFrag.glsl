#version 460 core

in vec2 vPosOnQuad;
in vec3 color;
in float iradius;

void main() {
    float dist = length(vPosOnQuad);
    if (dist > iradius) {
        discard;
    }
    FragColor = vec4(color, 1.0f);
}
