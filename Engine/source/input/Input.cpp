/*!------------------------------------------------------------------------------------------------
    File: Input.cpp
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
        ???
        - Created...

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// IsKeyDown/WasPressed, mouse, scroll (GLFW wrappe r)
#include "input/input.h"

std::vector<std::tuple<int, int, Input_action>> Input::keyboard_action;
std::vector<std::tuple<int, int, Input_action>> Input::mouse_action;

Input::Input(GLFWwindow* window) {
	//GLFWkeyfun glfwSetKeyCallback(GLFWwindow * window, GLFWkeyfun callback)
	//typedef void(* GLFWkeyfun) (GLFWwindow *window, int key, int scancode, int action, int mods)
	//GLFWkeyfun is a function pointer
	glfwSetKeyCallback(window, keyboard_input_callback); //setup keyboard input callback before main game loop
	glfwSetMouseButtonCallback(window, mouse_input_callback); //setup mouse input callback before main game loop

	detect_keyboard_event(window, GLFW_KEY_1, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_2, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_3, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_4, GLFW_PRESS);

	detect_keyboard_event(window, GLFW_KEY_W, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_S, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_A, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_D, GLFW_REPEAT);
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

std::vector<std::tuple<int, int, Input_action>>& Input::GET_keyboard() {
	return keyboard_action;
}

void Input::detect_keyboard_event(GLFWwindow* window, int key, int event_action) {
	bool key_event_exist{ false };
	for (auto& [tuple_key,tuple_action,tuple_state] : keyboard_action) {
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
	//std::cout << keyboard_action.size() << "\n";
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); //Pass keyboard input to ImGui
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard && action == GLFW_PRESS) {
        logger.log("IMGUI wants to use the keyboard");
    }

    //key is affected by keyboard layout based on their types
    //scancode works for all keyboard layout regardless of their types
    //mods is a combination of modifier keys that are held down when the key event occurred
    else { //store my game keyboard input state here
		static int track_number{ -1 };
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (tuple_key == key) {

				switch (key) {
				case GLFW_KEY_1:
					if (action == event_action) {
						track_number = key;
						key_action = Input_action::take_control;
					}
					break;

				case GLFW_KEY_2:
					if (action == event_action) {
						track_number = key;
						key_action = Input_action::take_control;
					}
					break;

				case GLFW_KEY_3:
					if (action == event_action) {
						track_number = key;
						key_action = Input_action::take_control;
					}
					break;

				case GLFW_KEY_4:
					if (action == event_action) {
						track_number = key;
						key_action = Input_action::take_control;
					}
					break;

				case GLFW_KEY_W:
					if (action == static_cast<int>(event_action)) {
						key_action = Input_action::move_up;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_S:
					if (action == static_cast<int>(event_action)) {
						key_action = Input_action::move_down;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_A:
					if (action == static_cast<int>(event_action)) {
						key_action = Input_action::move_left;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_D:
					if (action == static_cast<int>(event_action)) {
						key_action = Input_action::move_right;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;
				}
			}
		}

		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::take_control) {
				if (tuple_key != track_number) {
					key_action = Input_action::no_action;
				}
			}
		}


            //if (pair.first == key) { //the key trigger by user is registered inside this keyboard_action vector
            //    switch (pair.second) {
            //    case Input_state::press:
            //        if (action == GLFW_PRESS) {
            //            logger.log("player press a specific key");
            //        }
            //        break;

            //    case Input_state::release:
            //        if (action == GLFW_RELEASE) {
            //            logger.log("player release a specific key");
            //        }
            //        break;

            //    case Input_state::hold:
            //        if (action == GLFW_REPEAT) {
            //            logger.log("player holds down a specific key");
            //        }
            //        break;
            //    }
            //}
    }
}

void Input::mouse_input_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); //Pass mouse input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse && action == GLFW_PRESS) {
		logger.log("IMGUI wants to use the mouse");
	}

	//else { //store my game mouse input state here
		//for (auto& [tuple_mouse, tuple_action, tuple_state] : mouse_action) {
		//	if (tuple_mouse == button) { //the button trigger by user is registered inside this mouse_action vector
		//		switch (tuple_action) {
		//		case Input_state::press:
		//			if (action == GLFW_PRESS) {
		//				logger.log("player press the mouse");
		//			}
		//			break;

		//		case Input_state::release:
		//			if (action == GLFW_RELEASE) {
		//				logger.log("player release the mouse");
		//			}
		//			break;
		//		}
		//	}
		//}
	//}
}

// maps top-row 0..9 and numpad 0..9 to a digit, else -1
//inline int keyToDigit(int key) {
//    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)   return key - GLFW_KEY_0;
//    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) return key - GLFW_KEY_KP_0;
//    return -1;
//}


//void Input::initSpriteScales(size_t textureCount) {
//    spriteCount = (textureCount > 1) ? (textureCount - 1) : 0;
//    spriteScales.assign(textureCount, 1.0f);                // [0]=bg, [1-spriteCount]=sprites
//    if (spriteCount == 0) selectedSprite = -1;              // none if no sprites
//    else if (selectedSprite > (int)spriteCount) selectedSprite = -1;
//}

static inline void printSel(int s) {
    if (s < 0) std::cout << "Selected sprite: NONE\n";
    else if (s == 0) std::cout << "Selected sprite: ALL\n";
    else std::cout << "Selected sprite: " << s << "\n";
}

//void Input::keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
//    ImGuiIO& io = ImGui::GetIO();
//    if (io.WantCaptureKeyboard) return;
//
//    void* up = glfwGetWindowUserPointer(window);
//    if (up == NULL) return;
//    if (action != GLFW_PRESS) return;
//
//    // Use a reference so we can write self.x (no )
//    Input& self = *static_cast<Input*>(up);
//
//    int digit = keyToDigit(key);
//    if (digit != -1) {
//        // 0 toggles ALL / NONE only if we actually have sprites
//        if (digit == 0) {
//            if (self.spriteCount > 0) {
//                self.selectedSprite = (self.selectedSprite == 0) ? -1 : 0;
//                printSel(self.selectedSprite);
//            }
//            return;
//        }
//
//        // 1..9 (or more in future): act ONLY if that sprite exists; no clamping
//        if ((size_t)digit <= self.spriteCount) {
//            self.selectedSprite = (self.selectedSprite == digit) ? -1 : digit; // toggle
//            printSel(self.selectedSprite);
//        }
//        // else: ignore digits beyond spriteCount
//        return;
//    }
//
//    //  0 = toggle ALL / NONE (only if we have sprites) 
//    if (key == GLFW_KEY_0) {
//        if (self.spriteCount > 0) {
//            if (self.selectedSprite == 0) self.selectedSprite = -1;  // deselect ALL / NONE
//            else self.selectedSprite = 0;                             // select ALL
//            if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
//            else std::cout << "Selected sprite: NONE\n";
//        }
//        return;
//    }
//
//    //  number keys 1-9 toggle that sprite if it exists
//    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
//        int want = key - GLFW_KEY_0; // 1-9
//        if ((size_t)want <= self.spriteCount) {
//            if (self.selectedSprite == want) {
//                self.selectedSprite = -1;  // toggle off - NONE
//                std::cout << "Selected sprite: NONE\n";
//            }
//            else {
//                self.selectedSprite = want;
//                std::cout << "Selected sprite: " << self.selectedSprite << "\n";
//            }
//        }
//        return;
//    }
//
//
//    if (key == GLFW_KEY_RIGHT) {
//        if (self.spriteCount > 0) {
//            if (self.selectedSprite < 0) self.selectedSprite = 0; // NONE 4 ALL
//            else if (self.selectedSprite < (int)self.spriteCount) self.selectedSprite += 1; 
//            else self.selectedSprite = -1; 
//            if (self.selectedSprite < 0) std::cout << "Selected sprite: NONE\n";
//            else if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
//            else std::cout << "Selected sprite: " << self.selectedSprite << "\n";
//        }
//        return;
//    }
//    if (key == GLFW_KEY_LEFT) {
//        if (self.spriteCount > 0) {
//            if (self.selectedSprite < 0) self.selectedSprite = (int)self.spriteCount; 
//            else if (self.selectedSprite == 0) self.selectedSprite = -1;
//            else self.selectedSprite -= 1;
//            if (self.selectedSprite < 0) std::cout << "Selected sprite: NONE\n";
//            else if (self.selectedSprite == 0) std::cout << "Selected sprite: ALL\n";
//            else std::cout << "Selected sprite: " << self.selectedSprite << "\n";
//        }
//        return;
//    }
//
//    // scale DOWN ( -  or keypad - ) 
//    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
//        if (self.spriteCount == 0) return;
//
//        if (self.selectedSprite == 0) {
//            size_t i = 1;
//            while (i <= self.spriteCount && i < self.spriteScales.size()) {
//                float& s = self.spriteScales[i];
//                s -= 0.1f;
//                if (s < 0.2f) s = 0.2f;
//                ++i;
//            }
//           if (self.spriteCount >= 1 && self.spriteScales.size() > 1)
//                std::cout << "Scale[ALL] = " << self.spriteScales[1] << "\n";
//            return;
//        }
//        else if (self.selectedSprite >= 1 &&
//            (size_t)self.selectedSprite < self.spriteScales.size()) {
//            float& s = self.spriteScales[self.selectedSprite];
//            s -= 0.1f;
//            if (s < 0.2f) s = 0.2f;
//            s = std::round(s * 10.0f) / 10.0f;                    
//            std::cout << "Scale[" << self.selectedSprite << "] = "
//                << std::fixed << std::setprecision(1) << s << "\n";
//
//        }
//        else {
//            std::cout << "No sprite selected.\n";
//        }
//        return;
//    }
//
//    // scale UP ( =/+  or keypad + )
//    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
//        if (self.spriteCount == 0) return;
//
//        if (self.selectedSprite == 0) {
//            size_t i = 1;
//            while (i <= self.spriteCount && i < self.spriteScales.size()) {
//                float& s = self.spriteScales[i];
//                s += 0.1f;
//                if (s > 3.0f) s = 3.0f;
//                ++i;
//            }
//            float shown = (self.spriteScales.size() > 1) ? self.spriteScales[1] : 0.f;
//            shown = std::round(shown * 10.0f) / 10.0f;
//            if (self.spriteCount >= 1 && self.spriteScales.size() > 1)
//                std::cout << "Scale[ALL] = " << std::fixed << std::setprecision(1) << shown << "\n";
//            return;
//        }
//        else if (self.selectedSprite >= 1 &&
//            (size_t)self.selectedSprite < self.spriteScales.size()) {
//            float& s = self.spriteScales[self.selectedSprite];
//            s += 0.1f;
//            if (s > 3.0f) s = 3.0f;
//            s = std::round(s * 10.0f) / 10.0f;                      // snap to 0.1 steps
//            std::cout << "Scale[" << self.selectedSprite << "] = "
//                << std::fixed << std::setprecision(1) << s << "\n";
//
//        }
//        else {
//            std::cout << "No sprite selected.\n";
//        }
//        return;
//    }
//
//    // your original WASD prints 
//        switch (key) {
//        case GLFW_KEY_W:
//            std::cout << "I am moving up\n";
//            break;
//
//        case GLFW_KEY_S:
//            std::cout << "I am moving down\n";
//            break;
//
//        case GLFW_KEY_A:
//            std::cout << "I am moving left\n";
//            break;
//
//        case GLFW_KEY_D:
//            std::cout << "I am moving right\n";
//            break;
//
//        case GLFW_KEY_ESCAPE:
//            //glfwDestroyWindow(window);
//            break;
//        }
//}
