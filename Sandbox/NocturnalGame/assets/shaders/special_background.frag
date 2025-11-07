#version 330 core

out vec4 fragment_color;
in vec2 texture_coordinates;

uniform sampler2D bgTexture;
uniform float time;        // for animation

void main()
{
    // Slight scroll over time for visual movement
    vec2 scroll = vec2(0.05 * time, 0.0);
    vec2 uv = texture_coordinates + scroll;

    fragment_color = texture(bgTexture, uv);
}

