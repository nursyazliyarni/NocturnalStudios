/* !------------------------------------------------------------------------------------------------
    File: Shader.cpp
    Project : NocturnalStudio

    This file contains functions that loads, compiles and links vertex file and fragment file into 
	a usable shader program to be used by renderer class for background image.

    Primary Author:
        Ang Yu Xuan (100%)
		-This file contains functions that loads, compiles and links vertex file.

    Secondary Author:
         Chang Qi Xuan Celest
        -Function to set uniform variable for vec3.

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "graphics/Shader.h"

Shader::Shader(const char* vertex_path, const char* fragment_path) {
	//Read vertex AND fragment shader source code from files
    // const char* vertex_Code, *fragment_Code;
    std::ifstream vertex_ShaderFile{ vertex_path };
    std::ifstream fragment_ShaderFile{ fragment_path };
    if (!vertex_ShaderFile || !fragment_ShaderFile) { //if file fails to open
        logger.error("Shader File cannot be opened!");
        return;
    }

    std::stringstream vertex_Shader_ss, fragment_Shader_ss;
    //rdbuf() copies the entire buffer contents at once into stringstream
    vertex_Shader_ss << vertex_ShaderFile.rdbuf();
    fragment_Shader_ss << fragment_ShaderFile.rdbuf();
    //close file stream
    vertex_ShaderFile.close(); fragment_ShaderFile.close();
    Compile_Shaders(vertex_Shader_ss.str().c_str(), fragment_Shader_ss.str().c_str());
}

Shader::Shader(const std::vector<std::pair<std::string, std::string>>& shader_filePATH_list) {
    for (const auto& [vertex_path, fragment_path] : shader_filePATH_list) {
        //Read vertex AND fragment shader source code from files
        const char* vertex_Code, * fragment_Code;
        std::ifstream vertex_ShaderFile{ vertex_path };
        std::ifstream fragment_ShaderFile{ fragment_path };
        if (!vertex_ShaderFile || !fragment_ShaderFile) { //if file fails to open
            logger.error("Shader File cannot be opened!");
            return;
            //throw std::runtime_error("Failed to open shader file: " + vertex_path + " or " + fragment_path);
        }

        std::stringstream vertex_Shader_ss, fragment_Shader_ss;
        //rdbuf() copies the entire buffer contents at once into stringstream
        vertex_Shader_ss << vertex_ShaderFile.rdbuf();
        fragment_Shader_ss << fragment_ShaderFile.rdbuf();
        //close file stream
        vertex_ShaderFile.close(); fragment_ShaderFile.close();
        Compile_Shaders(vertex_Shader_ss.str().c_str(), fragment_Shader_ss.str().c_str());
    }
}

Shader::~Shader() { 
    for (const auto& program_ID : program_obj) {
        glDeleteProgram(program_ID);
    }
}

Shader::Shader(Shader&& rhs_shader) noexcept :program_obj{ std::move(rhs_shader.program_obj) } { }//release ownership of rhs_shader

Shader& Shader::operator=(Shader&& rhs_shader) noexcept {
    if (this != &rhs_shader) { //prevent self assignment here
        for (const auto& program_ID : program_obj) {
            glDeleteProgram(program_ID);
        }
        program_obj = std::move(rhs_shader.program_obj); //this->program_obj steal resources
    }
    return *this;
}

void Shader::Compile_Shaders(const char* vertex_Code, const char* fragment_Code) {
    GLuint vertex_shader, fragment_shader; GLint check_status;
    //glCreateShader creates empty shader object
    //GL_VERTEX_SHADER runs on the programmable vertex processor
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource sets source code in vertex_shader from vertex_Code
    //passing 1 NULL-terminated string element to vertex_shader
    glShaderSource(vertex_shader, 1, &vertex_Code, NULL);
    glCompileShader(vertex_shader); //compile vertex_shader object
    //if return is true, it means occur has occured during compilation
    if (Check_Shaders_Errors(Shader_check_state::compile, vertex_shader, check_status)) { 
        return; //exit from this function immediately, 
    }

    //GL_FRAGMENT_SHADER runs on the programmable fragment processor
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_Code, NULL);
    glCompileShader(fragment_shader);
    //if return is true, it means occur has occured during compilation
    if (Check_Shaders_Errors(Shader_check_state::compile, fragment_shader, check_status)) {
        return; //exit from this function immediately, 
    }
    //we will ONLY link the program if no eccur occured for compilation of vertex_shader and fragment_shader
    Link_Program(vertex_shader,fragment_shader);
}

void Shader::Link_Program(GLuint &vertex_shader, GLuint &fragment_shader) {
    GLint check_status; GLuint program_ID;
    program_ID = glCreateProgram();//creates an empty program object
    //glAttachShader attaches shader object to the program object
    glAttachShader(program_ID, vertex_shader);
    glAttachShader(program_ID, fragment_shader);
    glLinkProgram(program_ID); //glLinkProgram links the program object
    //if return is true, it means occur has occured during linking
    if (Check_Shaders_Errors(Shader_check_state::link, program_ID, check_status)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return; //exit from this function immediately, 
    }
  
    //If we reach here, it means the process of loading->compilation->linking are all successful
    //the program is created successfully at this stage, deallocate all shader object now
    program_obj.push_back(program_ID);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

bool Shader::Check_Shaders_Errors(Shader_check_state check_input, GLuint &shader_obj, GLint &check_status) {
    if (check_input == Shader_check_state::compile) { //check if any error occurs while compiling
        glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &check_status);
        if (!check_status) { //if compile operation on vertex_shader/vertex_fragment fails
            GLint info_length{ 0 };
            glGetShaderiv(shader_obj, GL_INFO_LOG_LENGTH, &info_length);
            std::vector<GLchar>info_log(info_length);
            //use nullptr since number of char written to info_log is not important here
            glGetShaderInfoLog(shader_obj, info_length, nullptr, info_log.data());
            logger.error("Compilation fails here: " + static_cast<std::string>(info_log.data()));
            glDeleteShader(shader_obj); //deallocate vertex_shader/vertex_fragment object
            return true; //error has occured
        }
    }

    else { //check if any error occurs while linking
        glGetProgramiv(shader_obj, GL_LINK_STATUS, &check_status);
        if (!check_status) { //if link operation on program_obj fails
            GLint info_length{ 0 };
            glGetProgramiv(shader_obj, GL_INFO_LOG_LENGTH, &info_length);
            std::vector<GLchar>info_log(info_length);
            //use nullptr since number of char written to info_log is not important here
            glGetProgramInfoLog(shader_obj, info_length, nullptr, info_log.data());
            logger.error("Link operation fails here: " + static_cast<std::string>(info_log.data()));
            return true; //error has occured
        }
    }
    return false; //reach here means no error has occured
}

void Shader::user_Shader_program(unsigned int vector_index) {
    glUseProgram(program_obj[vector_index]); //The function tells OpenGL to use my shader program for all rendering
}

GLuint& Shader::get_programID(unsigned int vector_index) {
    return program_obj[vector_index];
}

//A single float uniform
void Shader::SET_uniform_variable(const GLchar* uniform_VARNAME, GLfloat uniform_value, unsigned int vector_index) {
    glUniform1f(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), uniform_value);
}

//A 2-component float vector(vec2)
void Shader::SET_uniform_variable(const GLchar* uniform_VARNAME, GLfloat uniform_value_v1, GLfloat uniform_value_v2, unsigned int vector_index) {
    glUniform2f(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), uniform_value_v1, uniform_value_v2);
}

//A single integer uniform
void Shader::SET_uniform_variable(const GLchar* uniform_VARNAME, GLint uniform_value, unsigned int vector_index) {
    glUniform1i(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), uniform_value);
}

//A 2-component int vector(vec2)
void Shader::SET_uniform_variable(const GLchar* uniform_VARNAME, GLint uniform_value_v1, GLint uniform_value_v2, unsigned int vector_index) {
    glUniform2i(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), uniform_value_v1, uniform_value_v2);
}

//A 1d-array of 1 OR 2-float component
void Shader::SET_arr_uniform_variable(const GLchar* uniform_VARNAME, GLsizei num_value, 
    const GLfloat* uniform_value_arr, arr_num_component num_component, unsigned int vector_index)
{
    //the user decides whether they want 1d-array of 1-float component OR 2-float component
    //size of array == value for each component * num of component
    if (num_component == arr_num_component::one_component) {
        glUniform1fv(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), num_value, uniform_value_arr);
    }

    else { //two-component
        glUniform2fv(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), num_value, uniform_value_arr);
    }
}

//A 1d-array of 1 OR 2-integer component
void Shader::SET_arr_uniform_variable(const GLchar* uniform_VARNAME, GLsizei num_value,
    const GLint* uniform_value_arr, arr_num_component num_component, unsigned int vector_index)
{
    //the user decides whether they want 1d-array of 1-integer component OR 2-integer component 
    if (num_component == arr_num_component::one_component) {
        glUniform1iv(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), num_value, uniform_value_arr);
    }

    else { //two-component
        glUniform2iv(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), num_value, uniform_value_arr);
    }
}

/**
* @brief Sets a vec3 uniform variable in the currently active shader program.
* This function uploads a 3-component floating-point vector (vec3) to a uniform
* variable in the GLSL shader. It retrieves the uniform location from the
* specified shader program index in `program_obj`.
* @param uniform_VARNAME Name of the uniform variable in the shader.
* @param v1 First (x) component of the vector.
* @param v2 Second (y) component of the vector.
* @param v3 Third (z) component of the vector.
* @param vector_index Index of the shader program in `program_obj` to which the uniform belongs.
* @note The shader program must be bound with `glUseProgram()` or
*       `user_Shader_program()` before calling this function.
* @see Shader::user_Shader_program()
* @see Shader::get_programID()
*/
void Shader::SET_uniform_variable(const GLchar* uniform_VARNAME, GLfloat v1, GLfloat v2, GLfloat v3, unsigned int vector_index) {
    glUniform3f(glGetUniformLocation(program_obj[vector_index], uniform_VARNAME), v1, v2, v3);
}