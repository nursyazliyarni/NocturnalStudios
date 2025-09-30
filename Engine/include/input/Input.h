// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)\

#pragma once

#define GLFW_INCLUDE_NONE 
#include "GLFW/glfw3.h"
#include "math/Vector.h"
#include "../imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include <iostream>

class Input {
public:
	Input();
	~Input();

	void process_user_input(GLFWwindow* window); 
	Vector2D& update_mouse_pos(GLFWwindow* window, bool want_normalize, int window_width, int window_height);
	Vector2D& GET_mouse_pos();

private:
	//Non-static member functions need an object to call it
	//So declare this function as static to avoid this issue
	//However the function belongs to the class instead
	static void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_input_callback(GLFWwindow* window, int button, int action, int mods);

	Vector2D& normalize_mouse_coor(Vector2D& mouse_pos, int window_width, int window_height);

private:
	//private data member
	Vector2D mouse_pos;
};