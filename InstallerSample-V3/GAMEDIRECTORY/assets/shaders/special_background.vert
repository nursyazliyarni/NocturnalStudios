#version 330 core

layout (location = 0) in vec3 inputPos;      // World-space position
layout (location = 1) in vec2 inputTexCoord;

out vec2 texture_coordinates;
uniform float parallaxFactor;

void main()
{
    // Apply a parallax scaling factor to simulate distant movement
    vec3 newPos = inputPos;
    newPos.xz *= parallaxFactor;

    gl_Position = vec4(newPos, 1.0);
    texture_coordinates = inputTexCoord;
}
