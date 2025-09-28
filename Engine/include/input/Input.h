// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)\

#pragma once

#define GLFW_INCLUDE_NONE 
#include "GLFW/glfw3.h"
#include "math/Vector.h"
#include "../imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"

enum class Input_state {
	none,press,release,hold
};

class Input {
public:
	Input(GLFWwindow* window);

	void update_mouse_pos(GLFWwindow* window);
	Vector2D GET_mouse_pos(bool want_normalize, int window_width, int window_height);
	Vector2D GET_mouse_delta();
	void process_user_input(GLFWwindow* window);

	void detect_keyboard_event(GLFWwindow* window, int key, Input_state event_action);
	void detect_mouse_event(GLFWwindow* window, int mouse, Input_state event_action);

private:
	//Non-static member functions need an object to call it
	//So declare this function as static to avoid this issue
	//However the function belongs to the class instead
	static void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_input_callback(GLFWwindow* window, int button, int action, int mods);

private:
	//private data member
	//mouse_pos[0] is previous mouse position, mouse_pos[1] is current mouse_position
	Vector2D mouse_pos[2];
	static std::vector<std::pair<int,Input_state>> keyboard_action;
	static std::vector<std::pair<int,Input_state>> mouse_action;
};