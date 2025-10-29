/*!------------------------------------------------------------------------------------------------
	File: Input.cpp
	Project: NocturnalStudio

	This file contains the platform window wrapper: creation/destruction of a GLFW window,
	OpenGL context setup (GLEW init), VSync control, event polling, buffer swap, and FPS-in-title helper.

	used for later physics and logic.

	Primary Author:
		 Ang Yu Xuan (55%)
		-implement functions for captures user keyboard input and mouse input and the respective
		callback functions. The callback functions will update the input action accordingly.

	Secondary Author:
		 Nur Syazliyarni (15%)
		-Created the messaging system for input

	Secondary Author:
		 Rishika Bimal Attodi  (35%)
		-Added sprite selection/deselection logic and the scaling/rotation key handling,
		plus per-sprite transform helpers (init/get pointers, rotation updates)

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)
#include "input/Input.h"

#include <unordered_set>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
//#include "../messaging/MessageBus.h"
//#include "../messaging/InputMessage.h"
#include "messaging/Message.h"
#include "messaging/InputMessage.h"


std::vector<std::tuple<int, int, Input_action>> Input::keyboard_action;
std::vector<std::tuple<int, int, Input_action>> Input::mouse_action;

static std::unordered_set<int> pressedKeysThisFrame;
static std::unordered_set<int> pressedMouseThisFrame;


Input::Input(GLFWwindow* window) {
	//GLFWkeyfun glfwSetKeyCallback(GLFWwindow * window, GLFWkeyfun callback)
	//typedef void(* GLFWkeyfun) (GLFWwindow *window, int key, int scancode, int action, int mods)
	//GLFWkeyfun is a function pointer
	glfwSetKeyCallback(window, keyboard_input_callback); //setup keyboard input callback before main game loop
	glfwSetMouseButtonCallback(window, mouse_input_callback); //setup mouse input callback before main game loop

	glfwSetWindowUserPointer(window, this);



	/*detect_keyboard_event(window, GLFW_KEY_1, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_2, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_3, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_4, GLFW_PRESS);*/

	detect_keyboard_event(window, GLFW_KEY_ESCAPE, GLFW_PRESS);


	//input key for movement of sprite
	detect_keyboard_event(window, GLFW_KEY_W, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_S, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_A, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_D, GLFW_PRESS);

	//input key for scaling sprite
	detect_keyboard_event(window, GLFW_KEY_U, GLFW_PRESS); // bigger
	detect_keyboard_event(window, GLFW_KEY_I, GLFW_PRESS); // smaller
	//input key for rotating sprite
	detect_keyboard_event(window, GLFW_KEY_J, GLFW_PRESS); //clockwise
	detect_keyboard_event(window, GLFW_KEY_K, GLFW_PRESS); //anti-clockwise

	detect_keyboard_event(window, GLFW_KEY_X, GLFW_PRESS); // reset transform

	detect_mouse_event(window, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS); //detect left mouse press
	detect_mouse_event(window, GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS); //detect right mouse press
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
	return { mouse_pos[1] - mouse_pos[0] };
}

std::vector<std::tuple<int, int, Input_action>>& Input::GET_keyboard() {
	return keyboard_action;
}

void Input::detect_keyboard_event(GLFWwindow* window, int key, int event_action) {
	bool key_event_exist{ false };
	for (auto& [tuple_key, tuple_action, tuple_state] : keyboard_action) {
		//we use this loop to check if key AND event_action already exist inside the vector
		if (tuple_key == key && tuple_action == event_action) {
			key_event_exist = true;
		}
	}

	//we will add element to the vector if key_event_exist == false
	if (!key_event_exist) {
		keyboard_action.push_back({ key,event_action,Input_action::no_action });
	}
}

void Input::detect_mouse_event(GLFWwindow* window, int mouse, int event_action) {
	bool mouse_event_exist{ false };
	for (auto& [tuple_mouse, tuple_action, tuple_state] : mouse_action) {
		//we use this loop to check if mouse AND event_action already exist inside the vector
		if (tuple_mouse == mouse && tuple_action == event_action) {
			mouse_event_exist = true;
		}
	}

	//we will add element to the vector if mouse_event_exist == false
	if (!mouse_event_exist) {
		mouse_action.push_back({ mouse,event_action,Input_action::no_action });
	}
}


void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (ImGui::GetCurrentContext())
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); // let ImGui see it

	if (action == GLFW_PRESS) pressedKeysThisFrame.insert(key);          // Input owns it
}



void Input::mouse_input_callback(GLFWwindow* window, int button, int action, int mods) {
	if (ImGui::GetCurrentContext())
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); // let ImGui see it

	if (action == GLFW_PRESS) pressedMouseThisFrame.insert(button);       // Input owns it

	auto* self = static_cast<Input*>(glfwGetWindowUserPointer(window));
	if (self && self->messageBus) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		bool pressed = (action == GLFW_PRESS);
		auto msg = std::make_unique<Messaging::ClickedMouseMessage>(button, pressed, (float)xpos, (float)ypos);
		//std::cout << "[MessageBus] Broadcasting key " << Me << " action " << action << std::endl;
		self->messageBus->Broadcast(std::move(msg));
	}
}



bool Input::WasPressed(int k) const {
	return pressedKeysThisFrame.count(k) > 0;
}
bool Input::WasMousePressed(int b) const {
	return pressedMouseThisFrame.count(b) > 0;
}
void Input::EndFrame() {
	pressedKeysThisFrame.clear();
	pressedMouseThisFrame.clear();
}


int Input::ConsumeSceneKey() const {
	if (WasPressed(GLFW_KEY_ESCAPE)) return 0;        // ESC
	for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k) {  // 1..9
		if (WasPressed(k)) return (k - GLFW_KEY_1) + 1;
	}
	return -1;
}



// return pointer (for renderer)
const float* Input::getSpriteRotationsPtr() const {
	if (spriteRotations.empty()) return nullptr;
	return spriteRotations.data();
}

void Input::updateRotations() {
	for (size_t i = 1; i < spriteRotations.size(); ++i) {
		if (spriteRotatingCW[i]) {
			spriteRotations[i] += 1.0f;    // 1 deg per frame (adjust speed if needed)
			if (spriteRotations[i] >= 360.0f) spriteRotations[i] -= 360.0f;
		}
		if (spriteRotatingCCW[i]) {
			spriteRotations[i] -= 1.0f;
			if (spriteRotations[i] < 0.0f) spriteRotations[i] += 360.0f;
		}
	}
}



