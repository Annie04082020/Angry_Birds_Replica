#version 410 core

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

uniform vec4 uColor;
uniform float uThickness;

void main() {
    fragColor = uColor;
}
