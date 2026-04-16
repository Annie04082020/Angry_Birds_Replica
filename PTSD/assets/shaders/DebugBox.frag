#version 410 core

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 fragColor;

uniform vec4 uColor = vec4(1,0,0,1);
uniform float uThickness = 0.02;

void main() {
    float t = uThickness;
    if (uv.x < t || uv.x > 1.0 - t || uv.y < t || uv.y > 1.0 - t) {
        fragColor = uColor;
    } else {
        discard;
    }
}
