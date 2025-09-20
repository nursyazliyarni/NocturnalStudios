#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "gl/glew.h"

class Shader {
public:
	Shader(const char* vertex_path, const char* fragment_path); //constructor reads and builds the shader
	void user_Shader_program();

private:
	//private member functions here
	void Compile_Shaders(const char* vertex_Code, const char* fragment_Code);
	void Link_Program(GLuint &vertex_shader, GLuint &fragment_shader);
	bool Check_Shaders_Errors(const char* check_input, GLuint &shader_obj, GLint &check_status);

    //private data members here
	GLuint program_obj;
};

#endif