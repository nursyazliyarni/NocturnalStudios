#version 330 core

layout(location = 0) in vec3 image_pos;
layout(location = 1) in vec2 image_TexCoord;

out vec2 TexCoord;

void main()
{
    gl_Position = vec4(image_pos, 1.0);
    TexCoord = image_TexCoord;
}
