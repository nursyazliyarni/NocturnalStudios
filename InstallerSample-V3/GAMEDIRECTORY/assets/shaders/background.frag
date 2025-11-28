#version 330 core //Declares the version of GLSL we're using (OpenGL 3.3)
in vec2 output_Tex_Coord;

out vec4 fragment_RGBA_color;
uniform sampler2D bg_texture;

void main() {
    // Flip the Y coordinate (0 ↔ 1)
    vec2 uv = vec2(output_Tex_Coord.x, 1.0 - output_Tex_Coord.y);
    fragment_RGBA_color = texture(bg_texture, uv);
}