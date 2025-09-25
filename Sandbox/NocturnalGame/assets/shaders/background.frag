#version 330 core //Declares the version of GLSL we're using (OpenGL 3.3)
in vec2 output_Tex_Coord;

out vec4 fragment_RGBA_color;
uniform sampler2D bg_texture;

void main() {
    //texture() returns vec4 containing the sampled texel color(RGBA)
    fragment_RGBA_color = texture(bg_texture, output_Tex_Coord);
}