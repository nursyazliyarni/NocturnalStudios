// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)
#include "input/input.h"

Input::Input() {
	
}

Input::~Input() {
	
}

Vector2D& Input::update_mouse_pos(GLFWwindow* window, bool want_normalize, int window_width, int window_height) {
	double mouse_Xpos, mouse_Ypos;
	glfwGetCursorPos(window, &mouse_Xpos, &mouse_Ypos);
	mouse_pos[0] = mouse_Xpos, mouse_pos[1] = mouse_Ypos;
	return want_normalize? normalize_mouse_coor(mouse_pos,window_width,window_height) : mouse_pos;
}

Vector2D& Input::normalize_mouse_coor(Vector2D& mouse_pos, int window_width, int window_height) {
	mouse_pos[0] /= window_width, mouse_pos[1] /= window_height;
	return mouse_pos;
}

Vector2D& Input::GET_mouse_pos() {
	return mouse_pos;
}

void Input::process_user_input(GLFWwindow* window) {
	//GLFWkeyfun glfwSetKeyCallback(GLFWwindow * window, GLFWkeyfun callback)
	//typedef void(* GLFWkeyfun) (GLFWwindow *window, int key, int scancode, int action, int mods)
	//GLFWkeyfun is a function pointer
	glfwSetKeyCallback(window, keyboard_input_callback); //handle keyboard input
	glfwSetMouseButtonCallback(window, mouse_input_callback); //handle mouse input
}

void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); //Pass keyboard input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) return;

	if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_W:
				std::cout << "I am moving up\n";
				break;

			case GLFW_KEY_S:
				std::cout << "I am moving down\n";
				break;

			case GLFW_KEY_A:
				std::cout << "I am moving left\n";
				break;

			case GLFW_KEY_D:
				std::cout << "I am moving right\n";
				break;
			
			case GLFW_KEY_ESCAPE:
				//glfwDestroyWindow(window);
				break;
		}
	}
}

void Input::mouse_input_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); //Pass mouse input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;
	
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "left mouse button is pressed\n";
			break;

		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "right mouse button is pressed\n";
			break;

		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "middle mouse button is pressed\n";
			break;
		}
	}
}