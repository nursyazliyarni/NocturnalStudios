// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)
#include "input/input.h"
extern float value;

std::vector<std::pair<int, Input_state>> Input::keyboard_action;
std::vector<std::pair<int, Input_state>> Input::mouse_action;

Input::Input(GLFWwindow* window) {
	//GLFWkeyfun glfwSetKeyCallback(GLFWwindow * window, GLFWkeyfun callback)
	//typedef void(* GLFWkeyfun) (GLFWwindow *window, int key, int scancode, int action, int mods)
	//GLFWkeyfun is a function pointer
	glfwSetKeyCallback(window, keyboard_input_callback); //setup keyboard input callback before main game loop
	glfwSetMouseButtonCallback(window, mouse_input_callback); //setup mouse input callback before main game loop
}

void Input::update_mouse_pos(GLFWwindow* window) {
	mouse_pos[0].x() = mouse_pos[1].x(), mouse_pos[0].y() = mouse_pos[1].y(); //update previous mouse position
	double mouse_Xpos, mouse_Ypos;
	glfwGetCursorPos(window, &mouse_Xpos, &mouse_Ypos);
	mouse_pos[1].x() = mouse_Xpos, mouse_pos[1].y() = mouse_Ypos; //update current mouse position
}

Vector2D Input::GET_mouse_pos(bool want_normalize, int window_width, int window_height) {
	if (want_normalize) { //the user want normalize mouse coordinates
		//normalize current mouse position
		return { mouse_pos[1].x() / window_width, mouse_pos[1].y() / window_height };
	}
	return mouse_pos[1];
}

Vector2D Input::GET_mouse_delta() {
	//current - previous-> how much the mouse move in current frame as compared to last frame
	return { mouse_pos[1] - mouse_pos[0]};
}

void Input::process_user_input(GLFWwindow* window) {
	
}

void Input::detect_keyboard_event(GLFWwindow* window, int key, Input_state event_action) {
	bool key_event_exist{ false };
	for (std::pair<int, Input_state>pair : keyboard_action) {
		//we use this loop to check if key AND event_action already exist inside the vector
		if (pair.first == key && pair.second == event_action) {
			key_event_exist = true;
		}
	}

	//we will add element to the vector if key_event_exist == false
	if (!key_event_exist) { 
		keyboard_action.push_back({ key,event_action });
	}
}

void Input::detect_mouse_event(GLFWwindow* window, int mouse, Input_state event_action) {
	bool mouse_event_exist{ false };
	for (std::pair<int, Input_state>pair : mouse_action) {
		//we use this loop to check if mouse AND event_action already exist inside the vector
		if (pair.first == mouse && pair.second == event_action) {
			mouse_event_exist = true;
		}
	}

	//we will add element to the vector if mouse_event_exist == false
	if (!mouse_event_exist) {
		mouse_action.push_back({ mouse,event_action });
	}
}

void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); //Pass keyboard input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard && action == GLFW_PRESS) {
		logger.log("IMGUI wants to use the keyboard");
	}

	//key is affected by keyboard layout based on their types
	//scancode works for all keyboard layout regardless of their types
	//mods is a combination of modifier keys that are held down when the key event occurred
	else { //store my game keyboard input state here
		for (std::pair<int, Input_state>pair : keyboard_action) {
			if (pair.first == key) { //the key trigger by user is registered inside this keyboard_action vector
				switch (pair.second) {
				case Input_state::press:
					if (action == GLFW_PRESS) {
						logger.log("player press unknown key");
						pair.second = Input_state::none;
					}
					break;

				case Input_state::release:
					if (action == GLFW_RELEASE) {
						logger.log("player release unknown key");
						pair.second = Input_state::none;
					}
					break;

				case Input_state::hold:
					if (action == GLFW_REPEAT) {
						logger.log("player holds down unknown key");
						pair.second = Input_state::none;
					}
					break;
				}
			}
		}
	}
}

void Input::mouse_input_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); //Pass mouse input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse && action == GLFW_PRESS) {
		logger.log("IMGUI wants to use the mouse");
	}
	
	else { //store my game mouse input state here
		for (std::pair<int, Input_state>pair : mouse_action) {
			if (pair.first == button) { //the button trigger by user is registered inside this mouse_action vector
				switch (pair.second) {
				case Input_state::press:
					if (action == GLFW_PRESS) {
						logger.log("player press unknown mouse");
						pair.second = Input_state::none;
					}
					break;

				case Input_state::release:
					if (action == GLFW_RELEASE) {
						logger.log("player release unknown mouse");
						pair.second = Input_state::none;
					}
					break;
				}
			}
		}
	}
}