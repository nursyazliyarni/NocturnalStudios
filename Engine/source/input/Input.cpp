/*!------------------------------------------------------------------------------------------------
    File: Input.cpp
    Project: NocturnalStudio

    This file contains the implementation of the ImGuiManager class, which manages the integration
    of the ImGui library into the customize game engine. It handles the initialization,
    frame updates, and rendering of ImGui components, as well as providing various UI elements
    for debugging and interaction within the engine.

    Primary Author:
        ???
        - Created...

    Secondary Author:
        Rishika Bimal Attodi
        - Integrated key adjustments dynamic scaling of sprites

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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




//void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); //Pass keyboard input to ImGui
//	ImGuiIO& io = ImGui::GetIO();
//	if (io.WantCaptureKeyboard) return;
//
//	if (action == GLFW_PRESS) {
//		switch (key) {
//			case GLFW_KEY_W:
//				std::cout << "I am moving up\n";
//				break;
//
//			case GLFW_KEY_S:
//				std::cout << "I am moving down\n";
//				break;
//
//			case GLFW_KEY_A:
//				std::cout << "I am moving left\n";
//				break;
//
//			case GLFW_KEY_D:
//				std::cout << "I am moving right\n";
//				break;
//			
//			case GLFW_KEY_ESCAPE:
//				//glfwDestroyWindow(window);
//				break;
//		}
//	}
//}

// Input.cpp
// Input.cpp

void Input::process_user_input(GLFWwindow* window) {
    glfwSetWindowUserPointer(window, this);                
    glfwSetKeyCallback(window, keyboard_input_callback);
    glfwSetMouseButtonCallback(window, mouse_input_callback);
}

void Input::initSpriteScales(size_t textureCount) {
    spriteCount = (textureCount > 1) ? (textureCount - 1) : 0;
    spriteScales.assign(textureCount, 1.0f);                // [0]=bg, [1-spriteCount]=sprites
    if (spriteCount == 0) selectedSprite = -1;              // none if no sprites
    else if (selectedSprite > (int)spriteCount) selectedSprite = -1;
}

static inline void printSel(int s) {
    if (s < 0) std::cout << "Selected sprite: NONE\n";
    else if (s == 0) std::cout << "Selected sprite: ALL\n";
    else std::cout << "Selected sprite: " << s << "\n";
}

void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) return;

    void* up = glfwGetWindowUserPointer(window);
    if (up == NULL) return;
    if (action != GLFW_PRESS) return;

    // Use a reference so we can write self.x (no )
    Input& self = *static_cast<Input*>(up);

    //  0 = toggle ALL / NONE (only if we have sprites) 
    if (key == GLFW_KEY_0) {
        if (self.spriteCount > 0) {
            if (self.selectedSprite == 0) self.selectedSprite = -1;  // deselect ALL / NONE
            else self.selectedSprite = 0;                             // select ALL
            if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
            else std::cout << "Selected sprite: NONE\n";
        }
        return;
    }

    //  number keys 1-9 toggle that sprite if it exists
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
        int want = key - GLFW_KEY_0; // 1-9
        if ((size_t)want <= self.spriteCount) {
            if (self.selectedSprite == want) {
                self.selectedSprite = -1;  // toggle off - NONE
                std::cout << "Selected sprite: NONE\n";
            }
            else {
                self.selectedSprite = want;
                std::cout << "Selected sprite: " << self.selectedSprite << "\n";
            }
        }
        return;
    }


    if (key == GLFW_KEY_RIGHT) {
        if (self.spriteCount > 0) {
            if (self.selectedSprite < 0) self.selectedSprite = 0; // NONE 4 ALL
            else if (self.selectedSprite < (int)self.spriteCount) self.selectedSprite += 1; 
            else self.selectedSprite = -1; 
            if (self.selectedSprite < 0) std::cout << "Selected sprite: NONE\n";
            else if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
            else std::cout << "Selected sprite: " << self.selectedSprite << "\n";
        }
        return;
    }
    if (key == GLFW_KEY_LEFT) {
        if (self.spriteCount > 0) {
            if (self.selectedSprite < 0) self.selectedSprite = (int)self.spriteCount; 
            else if (self.selectedSprite == 0) self.selectedSprite = -1;
            else self.selectedSprite -= 1;
            if (self.selectedSprite < 0) std::cout << "Selected sprite: NONE\n";
            else if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
            else std::cout << "Selected sprite: " << self.selectedSprite << "\n";
        }
        return;
    }

    // scale DOWN ( -  or keypad - ) 
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        if (self.spriteCount == 0) return;

        if (self.selectedSprite == 0) {
            size_t i = 1;
            while (i <= self.spriteCount && i < self.spriteScales.size()) {
                float& s = self.spriteScales[i];
                s -= 0.1f;
                if (s < 0.2f) s = 0.2f;
                ++i;
            }
           if (self.spriteCount >= 1 && self.spriteScales.size() > 1)
                std::cout << "Scale[ALL] = " << self.spriteScales[1] << "\n";
            return;
        }
        else if (self.selectedSprite >= 1 &&
            (size_t)self.selectedSprite < self.spriteScales.size()) {
            float& s = self.spriteScales[self.selectedSprite];
            s -= 0.1f;
            if (s < 0.2f) s = 0.2f;
            s = std::round(s * 10.0f) / 10.0f;                    
            std::cout << "Scale[" << self.selectedSprite << "] = "
                << std::fixed << std::setprecision(1) << s << "\n";

        }
        else {
            std::cout << "No sprite selected.\n";
        }
        return;
    }

    // scale UP ( =/+  or keypad + )
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        if (self.spriteCount == 0) return;

        if (self.selectedSprite == 0) {
            size_t i = 1;
            while (i <= self.spriteCount && i < self.spriteScales.size()) {
                float& s = self.spriteScales[i];
                s += 0.1f;
                if (s > 3.0f) s = 3.0f;
                ++i;
            }
            float shown = (self.spriteScales.size() > 1) ? self.spriteScales[1] : 0.f;
            shown = std::round(shown * 10.0f) / 10.0f;
            if (self.spriteCount >= 1 && self.spriteScales.size() > 1)
                std::cout << "Scale[ALL] = " << std::fixed << std::setprecision(1) << shown << "\n";
            return;
        }
        else if (self.selectedSprite >= 1 &&
            (size_t)self.selectedSprite < self.spriteScales.size()) {
            float& s = self.spriteScales[self.selectedSprite];
            s += 0.1f;
            if (s > 3.0f) s = 3.0f;
            s = std::round(s * 10.0f) / 10.0f;                      // snap to 0.1 steps
            std::cout << "Scale[" << self.selectedSprite << "] = "
                << std::fixed << std::setprecision(1) << s << "\n";

        }
        else {
            std::cout << "No sprite selected.\n";
        }
        return;
    }

    // your original WASD prints 
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
