/*!------------------------------------------------------------------------------------------------
	File: Input.h
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

// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)
#pragma once

#define GLFW_INCLUDE_NONE 
#include "GLFW/glfw3.h"
#include "math/Vector.h"
#include "../imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../messaging/MessageBus.h"
#include "../messaging/InputMessage.h"

enum class Input_action {
	no_action,take_control,
	move_up,move_down,move_left,move_right,
	magnify,minify,rotate_clockwise,rotate_anticlockwise, reset_transform, toggle_collision
};

class Input {
public:
	//RULE OF ZERO APPLY
	Input(GLFWwindow* window);

	void update_mouse_pos(GLFWwindow* window);
	Vector2D GET_mouse_pos(bool want_normalize, int window_width, int window_height);
	Vector2D GET_mouse_delta();
	std::vector<std::tuple<int, int, Input_action>>& GET_keyboard();

	void detect_keyboard_event(GLFWwindow* window, int key, int event_action);
	void detect_mouse_event(GLFWwindow* window, int mouse, int event_action);

	// initialize per-sprite scales to 1.0f for all textures
	void initSpriteScales(size_t textureCount);
	// pointer for renderer (nullptr if no sprites)

	void initSpriteRotations(size_t count);

	// expose raw pointers for Renderer
	const float* getSpriteScalesPtr() const {
		if (spriteScales.empty()) return nullptr;
		return spriteScales.data();
	}
	const float* getSpriteRotationsPtr() const;

	//update rotations every frame
	void updateRotations();

	// selection: -1 = NONE, 0 = ALL, 1=spriteCount = specific sprite
	int selectedSprite = -1;
	bool isDragging = false; //true if we are dragging a sprite with the mouse

private:
	//Non-static member functions need an object to call it
	//So declare this function as static to avoid this issue
	//However the function belongs to the class instead
	static void keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouse_input_callback(GLFWwindow* window, int button, int action, int mods);

	// number of sprites that can be scaled (we skip index 0 = background)
	size_t spriteCount = 0;

	// per-sprite scale values, sized dynamically to textureCount
	std::vector<float> spriteScales;

	std::vector<float> spriteRotations;  // per-sprite rotation angles (in degrees), current angle in degrees

    // toggles for continuous rotation
	std::vector<bool> spriteRotatingCW;     // clockwise
	std::vector<bool> spriteRotatingCCW;    // counter-clockwise



private:
	//private data member
	//mouse_pos[0] is previous mouse position, mouse_pos[1] is current mouse_position
	Vector2D mouse_pos[2];
	static std::vector<std::tuple<int,int,Input_action>> keyboard_action;
	static std::vector<std::tuple<int,int,Input_action>> mouse_action;
};

//Messaging namespace implemented for input system here
namespace Messaging 
{
	class InputSystemMessaging
	{
	public:
		InputSystemMessaging(MessageBus& bus, GLFWwindow* window)
			: bus(bus), window(window)
		{
			glfwSetWindowUserPointer(window, this);
			glfwSetKeyCallback(window, KeyCallback);
			//glfwSetCursorPosCallback(window, MouseMoveCallback);
			glfwSetMouseButtonCallback(window, MouseButtonCallback);
		}
	
	private:
        MessageBus& bus;
        GLFWwindow* window;

		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
			auto* self = static_cast<InputSystemMessaging*>(glfwGetWindowUserPointer(window));
			if (!self) return;

			auto msg = std::make_unique<KeyMessage>(key, action);
			self->bus.Broadcast(std::move(msg));
		}

		/*static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
			auto* self = static_cast<InputSystemMessaging*>(glfwGetWindowUserPointer(window));
			if (!self) return;

			auto msg = std::make_unique<MovingMouseMessage>((float)xpos, (float)ypos);
			self->bus.Broadcast(std::move(msg));
		}*/

		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
			auto* self = static_cast<InputSystemMessaging*>(glfwGetWindowUserPointer(window));
			if (!self) return;

			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);

			bool pressed = (action == GLFW_PRESS);
			auto msg = std::make_unique<ClickedMouseMessage>(button, pressed, (float)xpos, (float)ypos);
			self->bus.Broadcast(std::move(msg));
		}
    };
}