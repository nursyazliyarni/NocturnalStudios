#include "graphics/Shader.h"

Shader::Shader(const char* vertex_path, const char* fragment_path) :program_obj{ 0 } {
	//Read vertex AND fragment shader source code from files
    const char* vertex_Code, *fragment_Code;
    std::ifstream vertex_ShaderFile{ vertex_path };
    std::ifstream fragment_ShaderFile{ fragment_path };
    if (!vertex_ShaderFile || !fragment_ShaderFile) { //if file fails to open
        std::cerr << "File cannot be opened!\n";
        return;
    }

    std::stringstream vertex_Shader_ss, fragment_Shader_ss;
    //rdbuf() copies the entire buffer contents at once into stringstream
    vertex_Shader_ss << vertex_ShaderFile.rdbuf();
    fragment_Shader_ss << fragment_ShaderFile.rdbuf();
    //close file stream
    vertex_ShaderFile.close(); fragment_ShaderFile.close();
    std::cout << "1\n";
    Compile_Shaders(vertex_Shader_ss.str().c_str(), fragment_Shader_ss.str().c_str());
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
    if (Check_Shaders_Errors("Compile", vertex_shader, check_status)) { 
        return; //exit from this function immediately, 
    }

    //GL_FRAGMENT_SHADER runs on the programmable fragment processor
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_Code, NULL);
    glCompileShader(fragment_shader);
    //if return is true, it means occur has occured during compilation
    if (Check_Shaders_Errors("Compile", fragment_shader, check_status)) {
        return; //exit from this function immediately, 
    }
    std::cout << "2\n";
    //we will ONLY link the program if no eccur occured for compilation of vertex_shader and fragment_shader
    Link_Program(vertex_shader,fragment_shader);
}

void Shader::Link_Program(GLuint &vertex_shader, GLuint &fragment_shader) {
    GLint check_status;
    program_obj = glCreateProgram();//creates an empty program object
    //glAttachShader attaches shader object to the program object
    glAttachShader(program_obj, vertex_shader);
    glAttachShader(program_obj, fragment_shader);
    glLinkProgram(program_obj); //glLinkProgram links the program object
    //if return is true, it means occur has occured during linking
    if (Check_Shaders_Errors("Link", program_obj, check_status)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return; //exit from this function immediately, 
    }
  
    //If we reach here, it means the process of loading->compilation->linking are all successful
    //the program is created successfully at this stage, deallocate all shader object now
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    std::cout << "3\n";
}

bool Shader::Check_Shaders_Errors(const char* check_input, GLuint &shader_obj, GLint &check_status) {
    if (check_input != "Link") { //check if any error occurs while compiling
        glGetShaderiv(shader_obj, GL_COMPILE_STATUS, &check_status);
        if (!check_status) { //if compile operation on vertex_shader/vertex_fragment fails
            GLint info_length{ 0 };
            glGetShaderiv(shader_obj, GL_INFO_LOG_LENGTH, &info_length);
            std::vector<GLchar>info_log(info_length);
            //use nullptr since number of char written to info_log is not important here
            glGetShaderInfoLog(shader_obj, info_length, nullptr, info_log.data());
            std::cerr << "Compilation fails here:" << info_log.data() << "\n";
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
            std::cerr << "Link operation fails here:" << info_log.data() << "\n";
            return true; //error has occured
        }
    }
    return false; //reach here means no error has occured
}

void Shader::user_Shader_program() {
    glUseProgram(program_obj); //The function tells OpenGL to use my shader program for all rendering
}