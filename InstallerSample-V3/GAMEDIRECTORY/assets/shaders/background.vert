#version 330 core //Declares the version of GLSL we're using (OpenGL 3.3)
//the keyword in is used to declare all the input vertex attributes in the vertex shader
//vec2 is GLSL built-in types, vec2->2-component floating point vector
//ivec2->int, bvec2->boolean, uvec2->uint
//shader location must match CPU attribute index
layout (location = 0) in vec2 input_vert_pos; //vertex position
layout (location = 1) in vec2 input_Texture_Coord; //texture coordinate

//output<-vert become input->fragment
out vec2 output_Tex_Coord;

void main() {
    //gl_Position is the output of the vertex shader
    gl_Position = vec4(input_vert_pos.x, input_vert_pos.y, 0.0, 1.0);
    //output_color = input_vert_color;
    output_Tex_Coord = input_Texture_Coord;
}