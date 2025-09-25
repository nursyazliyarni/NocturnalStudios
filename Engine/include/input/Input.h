// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)\

#define GLFW_INCLUDE_NONE 
#include "GLFW/glfw3.h"
#include <iostream>

class Input {
public:
	Input();
	~Input();

	void process_user_input(GLFWwindow* window);

private:
	//Non-static member functions need an object to call it
	//So declare this function as static to avoid this issue
	//However the function belongs to the class instead
	static void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_input_callback(GLFWwindow* window, int button, int action, int mods);
};