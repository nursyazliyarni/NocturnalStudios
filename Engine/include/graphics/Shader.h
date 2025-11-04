/*!------------------------------------------------------------------------------------------------
	File: Shader.h
	Project: NocturnalStudio

	This file contains ...

	Primary Author:
		 Name (%)
		-Brief

	Secondary Author:
		 Name (%)
		-Brief

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#ifndef SHADER_H
#define SHADER_H
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "core/Logger.h"
#include "gl/glew.h"

extern Logger logger;

enum class Shader_check_state {
	compile, link
};

enum class arr_num_component {
	one_component, two_component,
};

class Shader {
public:
	//constructor reads and builds the shader (vertex path follow by fragment path)
	Shader(const std::vector<std::pair<std::string,std::string>>& shader_filePATH_list);
	~Shader(); //destructor for glDeleteProgram(program_obj);
	//I don't want 2 shader object sharing the same program_obj, IT IS DANGEROUS
	//when doing glDeleteProgram(program_obj)
	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;
	Shader(Shader&& rhs_shader) noexcept; //move constructor
	Shader& operator=(Shader&& rhs_Shader) noexcept; //move assignment

	void user_Shader_program(unsigned int vector_index);
	GLuint& get_programID(unsigned int vector_index);

	//function overload for SET_uniform_variable
	//The compiler will choose which function to call based on input parameter type
	//A single float uniform
	void SET_uniform_variable(const GLchar* uniform_VARNAME, GLfloat uniform_value, unsigned int vector_index);
	//A 2-component float vector(vec2)
	void SET_uniform_variable(const GLchar* uniform_VARNAME, GLfloat uniform_value_v1, GLfloat uniform_value_v2, unsigned int vector_index);
	//A single integer uniform
	void SET_uniform_variable(const GLchar* uniform_VARNAME, GLint uniform_value, unsigned int vector_index);
	//A 2-component integer vector(vec2)
	void SET_uniform_variable(const GLchar* uniform_VARNAME, GLint uniform_value_v1, GLint uniform_value_v2, unsigned int vector_index);

	//A 1d-array of 1 OR 2-float component
	void SET_arr_uniform_variable(const GLchar* uniform_VARNAME, GLsizei num_value, 
		const GLfloat* uniform_value_arr, arr_num_component num_component, unsigned int vector_index);
	//A 1d-array of 1 OR 2-integer component
	void SET_arr_uniform_variable(const GLchar * uniform_VARNAME, GLsizei num_value,
		const GLint * uniform_value_arr, arr_num_component num_component, unsigned int vector_index);

private:
	//private member functions here
	void Compile_Shaders(const char* vertex_Code, const char* fragment_Code);
	void Link_Program(GLuint &vertex_shader, GLuint &fragment_shader);
	bool Check_Shaders_Errors(Shader_check_state check_input, GLuint &shader_obj, GLint &check_status);

    //private data members here
	std::vector<GLuint> program_obj;
};

#endif