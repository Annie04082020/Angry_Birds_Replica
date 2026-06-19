#version 410 core

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

uniform sampler2D surface;
uniform float opacity;
uniform vec3 colorTint;
uniform float grayscaleAmount;

void main() {
    vec4 texColor = texture(surface, uv);

    if (texColor.a < 0.01)
        discard;

    vec3 tintedColor = texColor.rgb * colorTint;
    float luminance = dot(tintedColor, vec3(0.299, 0.587, 0.114));
    vec3 finalColor = mix(tintedColor, vec3(luminance), grayscaleAmount);
    fragColor = vec4(finalColor, texColor.a * opacity);
}
