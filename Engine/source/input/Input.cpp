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

	//input key to decide which sprite to control
	detect_keyboard_event(window, GLFW_KEY_1, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_2, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_3, GLFW_PRESS);
	detect_keyboard_event(window, GLFW_KEY_4, GLFW_PRESS);

	//input key for movement of sprite
	detect_keyboard_event(window, GLFW_KEY_W, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_S, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_A, GLFW_REPEAT);
	detect_keyboard_event(window, GLFW_KEY_D, GLFW_REPEAT);

	//input key for scaling sprite
	detect_keyboard_event(window, GLFW_KEY_U, GLFW_REPEAT); //bigger
	detect_keyboard_event(window, GLFW_KEY_I, GLFW_REPEAT); //smaller
	//input key for rotating sprite
	detect_keyboard_event(window, GLFW_KEY_J, GLFW_REPEAT); //clockwise
	detect_keyboard_event(window, GLFW_KEY_K, GLFW_REPEAT); //anti-clockwise
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
		static int track_number{ -1 }; //this is to track which key is pressed last time
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
					if (action == event_action) {
						key_action = Input_action::move_up;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_S:
					if (action == event_action) {
						key_action = Input_action::move_down;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_A:
					if (action == event_action) {
						key_action = Input_action::move_left;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_D:
					if (action == event_action) {
						key_action = Input_action::move_right;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_U:
					if (action == event_action) {
						key_action = Input_action::magnify;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_I:
					if (action == event_action) {
						key_action = Input_action::minify;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_J:
					if (action == event_action) {
						key_action = Input_action::rotate_clockwise;
					}

					else {
						key_action = Input_action::no_action;
					}
					break;

				case GLFW_KEY_K:
					if (action == event_action) {
						key_action = Input_action::rotate_anticlockwise;
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
    }
}

void Input::mouse_input_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods); //Pass mouse input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse && action == GLFW_PRESS) {
		logger.log("IMGUI wants to use the mouse");
	}
}

//void Input::initSpriteScales(size_t textureCount) {
//    spriteCount = (textureCount > 1) ? (textureCount - 1) : 0;
//    spriteScales.assign(textureCount, 1.0f);                // [0]=bg, [1-spriteCount]=sprites
//    if (spriteCount == 0) selectedSprite = -1;              // none if no sprites
//    else if (selectedSprite > (int)spriteCount) selectedSprite = -1;
//}
//    if (textureCount > 1) {
//        spriteCount = textureCount - 1;
//    }
//    else {
//        spriteCount = 0;
//    }
//
//    spriteScales.assign(textureCount, 1.0f);     // all scales = 1.0
//    spriteRotations.assign(textureCount, 0.0f);  // all rotations = 0.0
//
//    if (spriteCount == 0) {
//        selectedSprite = -1;
//    }
//    else if (selectedSprite > (int)spriteCount) {
//        selectedSprite = -1;
//    }
//}


void Input::initSpriteScales(size_t textureCount) {
    if (textureCount > 1) {
        spriteCount = textureCount - 1;   // skip background
    }
    else {
        spriteCount = 0;
    }

    // assign default values
    spriteScales.assign(textureCount, 1.0f);      // all scales = 1.0
    spriteRotations.assign(textureCount, 0.0f);   // all rotations = 0 deg
    spriteRotatingCW.assign(textureCount, false); // all toggles off
    spriteRotatingCCW.assign(textureCount, false);

    if (spriteCount == 0) {
        selectedSprite = -1;
    }
    else if (selectedSprite > (int)spriteCount) {
        selectedSprite = -1;
    }
}




static inline void printSel(int s) {
    if (s < 0) std::cout << "Selected sprite: NONE\n";
    else if (s == 0) std::cout << "Selected sprite: ALL\n";
    else std::cout << "Selected sprite: " << s << "\n";
}


void Input::initSpriteRotations(size_t count) {
    spriteRotations.assign(count, 0.0f);       // all start at 0 deg
    spriteRotatingCW.assign(count, false);     // not rotating initially
    spriteRotatingCCW.assign(count, false);
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
//    // Toggle counter-clockwise rotation (Q key)
//    if (key == GLFW_KEY_R) {
//        if (self.spriteCount > 0) {
//            if (self.selectedSprite == 0) {
//                // ALL mode ? check if currently rotating CCW
//                bool allCCW = true;
//                for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCCW.size(); ++i) {
//                    if (!self.spriteRotatingCCW[i]) {
//                        allCCW = false;
//                        break;
//                    }
//                }
//
//                if (allCCW) {
//                    // If already CCW, turn OFF
//                    for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCCW.size(); ++i) {
//                        self.spriteRotatingCCW[i] = false;
//                    }
//                    std::cout << "All sprites CCW = OFF\n";
//                }
//                else {
//                    // Else turn CCW ON and CW OFF
//                    for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCCW.size(); ++i) {
//                        self.spriteRotatingCCW[i] = true;
//                        self.spriteRotatingCW[i] = false;
//                    }
//                    std::cout << "All sprites CCW = ON\n";
//                }
//            }
//            else if (self.selectedSprite >= 1 &&
//                (size_t)self.selectedSprite < self.spriteRotatingCCW.size()) {
//                bool nowOn = !self.spriteRotatingCCW[self.selectedSprite];
//                self.spriteRotatingCCW[self.selectedSprite] = nowOn;
//                self.spriteRotatingCW[self.selectedSprite] = false;
//                std::cout << "Sprite[" << self.selectedSprite << "] CCW = "
//                    << (nowOn ? "ON" : "OFF") << "\n";
//            }
//        }
//        return;
//    }
//
//    // Toggle clockwise rotation (E key)
//    if (key == GLFW_KEY_T) {
//        if (self.spriteCount > 0) {
//            if (self.selectedSprite == 0) {
//                // ALL mode ? check if currently rotating CW
//                bool allCW = true;
//                for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCW.size(); ++i) {
//                    if (!self.spriteRotatingCW[i]) {
//                        allCW = false;
//                        break;
//                    }
//                }
//
//                if (allCW) {
//                    // If already CW, turn OFF
//                    for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCW.size(); ++i) {
//                        self.spriteRotatingCW[i] = false;
//                    }
//                    std::cout << "All sprites CW = OFF\n";
//                }
//                else {
//                    // Else turn CW ON and CCW OFF
//                    for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotatingCW.size(); ++i) {
//                        self.spriteRotatingCW[i] = true;
//                        self.spriteRotatingCCW[i] = false;
//                    }
//                    std::cout << "All sprites CW = ON\n";
//                }
//            }
//            else if (self.selectedSprite >= 1 &&
//                (size_t)self.selectedSprite < self.spriteRotatingCW.size()) {
//                bool nowOn = !self.spriteRotatingCW[self.selectedSprite];
//                self.spriteRotatingCW[self.selectedSprite] = nowOn;
//                self.spriteRotatingCCW[self.selectedSprite] = false;
//                std::cout << "Sprite[" << self.selectedSprite << "] CW = "
//                    << (nowOn ? "ON" : "OFF") << "\n";
//            }
//        }
//        return;
//    }
//
//
//    // Reset all sprites (O key)
//    if (key == GLFW_KEY_X) {
//        if (self.spriteCount > 0) {
//            // Reset scales
//            for (size_t i = 1; i <= self.spriteCount && i < self.spriteScales.size(); ++i) {
//                self.spriteScales[i] = 1.0f;
//            }
//
//            // Reset rotations
//            for (size_t i = 1; i <= self.spriteCount && i < self.spriteRotations.size(); ++i) {
//                self.spriteRotations[i] = 0.0f;
//                self.spriteRotatingCW[i] = false;
//                self.spriteRotatingCCW[i] = false;
//            }
//
//            std::cout << "All sprites reset to original position, size, and rotation.\n";
//        }
//    }
//}
