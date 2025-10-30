#version 330 core //Declares the version of GLSL we're using (OpenGL 3.3)

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D backgroundTexture;
uniform vec3 tint_color;

void main()
{
    vec4 texture_color = texture(backgroundTexture, TexCoord);
    FragColor = vec4(texture_color.rgb * tint_color, texture_color.a);
}
