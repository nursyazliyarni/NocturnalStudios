/*!------------------------------------------------------------------------------------------------
	File: Input.cpp
	Project: NocturnalStudio

	This file contains the platform window wrapper: creation/destruction of a GLFW window,
	OpenGL context setup (GLEW init), VSync control, event polling, buffer swap, and FPS-in-title helper.


	Primary Author:
		???
		- Created...

	Secondary Author:
		Rishika Bimal Attodi
		- Implemented sprite selection/deselection and sprite rotation/scaling input handling.

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

	glfwSetWindowUserPointer(window, this);

	//input key to decide which sprite to control
	for (int k = GLFW_KEY_1; k <= GLFW_KEY_9; ++k) {
		detect_keyboard_event(window, k, GLFW_PRESS);
	}

	// (after the for-loop that calls detect_keyboard_event for 1 to 9
	detect_keyboard_event(window, GLFW_KEY_0, GLFW_PRESS);   // allow selecting ALL sprites


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
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods); //Pass keyboard input to ImGui
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard && action == GLFW_PRESS) {
		return;
	}

	//key is affected by keyboard layout based on their types
	//scancode works for all keyboard layout regardless of their types
	//mods is a combination of modifier keys that are held down when the key event occurred
	else { //store my game keyboard input state here
		static int track_number{ -1 }; //this is to track which key is pressed last time
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (tuple_key == key) {

				if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
					// get 'self' so we can read spriteCount
					void* uptr = glfwGetWindowUserPointer(window);
					Input* self = static_cast<Input*>(uptr);

					// compute the digit 1..9
					int digit = key - GLFW_KEY_0;

					if (action == event_action) {
						// only proceed if we have a valid self and at least that many sprites
						if (self != NULL && self->spriteCount > 0 &&
							digit >= 1 && static_cast<size_t>(digit) <= self->spriteCount) {

							// toggle behavior: if the same digit is already selected, deselect it
							if (track_number == key && key_action == Input_action::take_control) {
								key_action = Input_action::no_action;
								track_number = -1;

								std::stringstream ss;
								ss << "Sprite " << digit << " deselected";
								logger.log(ss.str());
							}
							else {
								// select this digit
								track_number = key;
								key_action = Input_action::take_control;

								// clear any other digit’s take_control flag
								for (size_t idx = 0; idx < keyboard_action.size(); ++idx) {
									int tuple_key2 = std::get<0>(keyboard_action[idx]);
									Input_action& key_action2 = std::get<2>(keyboard_action[idx]);

									if (tuple_key2 >= GLFW_KEY_1 && tuple_key2 <= GLFW_KEY_9) {
										if (tuple_key2 != key) {
											if (key_action2 == Input_action::take_control) {
												key_action2 = Input_action::no_action;
											}
										}
									}
								}

								std::stringstream ss;
								ss << "Sprite " << digit << " selected";
								logger.log(ss.str());
							}
						}
						else {
							// ===== small tweak: invalid digit ? force NONE and clear take_control =====
							track_number = -1;                          // NONE
							key_action = Input_action::no_action;     // ensure this tuple isn't active
							for (auto& t : keyboard_action) {
								Input_action& a = std::get<2>(t);
								if (a == Input_action::take_control) a = Input_action::no_action;
							}

							// keep your existing warning
							std::stringstream ss;
							ss << "Sprite " << digit << " does not exist (spriteCount = ";
							if (self != NULL) {
								ss << static_cast<unsigned long long>(self->spriteCount);
							}
							else {
								ss << 0ULL;
							}
							ss << ")";
							logger.warning(ss.str());
						}

						continue; // skip the switch for digit keys
					} // skip the switch for digit keys
				}

				// handle 0 = select ALL sprites
				// handle '0' as ALL
				// handle '0' = ALL (toggle on press)
				// handle '0' = ALL (toggle on press)
				// handle '0' = ALL (toggle on press)
				if (key == GLFW_KEY_0) {
            if (action == event_action) {
                if (track_number == GLFW_KEY_0) {
                    // ALL currently selected ? deselect to NONE
                    track_number = -1;
                    // clear any lingering take_control so Physics reads NONE
                    for (auto& t : keyboard_action) {
                        Input_action& a = std::get<2>(t);
                        if (a == Input_action::take_control) a = Input_action::no_action;
                    }
                    logger.log("ALL deselected (NONE) — rotations keep running, keys locked");
                } else {
                    // turn ALL ON
                    track_number = GLFW_KEY_0;
                    key_action   = Input_action::take_control;

                    // clear any digit’s take_control so ALL is the single active selection
                    for (auto& t : keyboard_action) {
                        int  k2 = std::get<0>(t);
                        auto& a2 = std::get<2>(t);
                        if (k2 >= GLFW_KEY_1 && k2 <= GLFW_KEY_9) {
                            if (a2 == Input_action::take_control) a2 = Input_action::no_action;
                        }
                    }
                    logger.log("Selected sprite: ALL");
                }
            }
            // ignore release (don’t clear on key-up)
            continue; // skip switch for digit keys
        }



				switch (key) {
				case GLFW_KEY_W:
				{
					if (action == GLFW_PRESS || action == GLFW_REPEAT) {
						key_action = Input_action::move_up;

						// Log once on initial press (not on repeats)
						if (action == GLFW_PRESS) {
							int selected = -1;
							if (track_number >= GLFW_KEY_1 && track_number <= GLFW_KEY_9) {
								selected = track_number - GLFW_KEY_0; // 1..9
							}

							std::stringstream ss;
							if (selected >= 1) {
								ss << "Sprite " << selected << " moving up";
							}
							else {
								ss << "Sprite moving up";
							}
							logger.log(ss.str());
						}
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}


				case GLFW_KEY_S:
				{
					// Press/repeat = move down; release = stop
					if (action == GLFW_PRESS || action == GLFW_REPEAT) {
						key_action = Input_action::move_down;

						// Log once on initial press (avoid spamming on repeat)
						if (action == GLFW_PRESS) {
							int selected = -1;
							// Convert last digit key (1..9) into a sprite number if valid
							if (track_number >= GLFW_KEY_1 && track_number <= GLFW_KEY_9) {
								selected = track_number - GLFW_KEY_0; // 1..9
							}

							std::stringstream ss;
							if (selected >= 1) {
								ss << "Sprite " << selected << " moving down";
							}
							else {
								ss << "Sprite moving down";
							}
							logger.log(ss.str());
						}
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}


				case GLFW_KEY_A:
				{
					// Press/repeat = move left; release = stop
					if (action == GLFW_PRESS || action == GLFW_REPEAT) {
						key_action = Input_action::move_left;

						// Log once on initial press (avoid spamming on repeat)
						if (action == GLFW_PRESS) {
							int selected = -1;

							// Convert last digit key (1..9) into a sprite number if valid
							if (track_number >= GLFW_KEY_1) {
								if (track_number <= GLFW_KEY_9) {
									selected = track_number - GLFW_KEY_0; // 1..9
								}
							}

							std::stringstream ss;
							if (selected >= 1) {
								ss << "Sprite " << selected << " moving left";
							}
							else {
								ss << "Sprite moving left";
							}
							logger.log(ss.str());
						}
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}

				case GLFW_KEY_D:
				{
					// Press/repeat = move right; release = stop
					if (action == GLFW_PRESS || action == GLFW_REPEAT) {
						key_action = Input_action::move_right;

						// Log once on initial press (avoid spamming on repeat)
						if (action == GLFW_PRESS) {
							int selected = -1;

							// Convert last digit key (1..9) into a sprite number if valid
							if (track_number >= GLFW_KEY_1) {
								if (track_number <= GLFW_KEY_9) {
									selected = track_number - GLFW_KEY_0; // 1..9
								}
							}

							std::stringstream ss;
							if (selected >= 1) {
								ss << "Sprite " << selected << " moving right";
							}
							else {
								ss << "Sprite moving right";
							}
							logger.log(ss.str());
						}
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}

				case GLFW_KEY_U:
				{
					// One press = request a single scale UP step
					if (action == GLFW_PRESS) {
						key_action = Input_action::magnify;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}

				case GLFW_KEY_I:
				{
					// One press = request a single scale DOWN step
					if (action == GLFW_PRESS) {
						key_action = Input_action::minify;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}




				case GLFW_KEY_J:
				{
					// only allow when some sprite (1..9) or ALL (0) is selected
					bool hasSelection = (track_number == GLFW_KEY_0) ||
						(track_number >= GLFW_KEY_1 && track_number <= GLFW_KEY_9);

					if (hasSelection && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
						key_action = Input_action::rotate_clockwise;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}

				case GLFW_KEY_K:
				{
					bool hasSelection = (track_number == GLFW_KEY_0) ||
						(track_number >= GLFW_KEY_1 && track_number <= GLFW_KEY_9);

					if (hasSelection && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
						key_action = Input_action::rotate_anticlockwise;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}

				case GLFW_KEY_X:
				{
					// Only allow reset when we have a valid selection (ALL or existing digit)
					void* up = glfwGetWindowUserPointer(window);
					Input* self = static_cast<Input*>(up);

					bool hasSelection = false;
					if (track_number == GLFW_KEY_0) {
						hasSelection = (self && self->spriteCount > 0);
					}
					else if (track_number >= GLFW_KEY_1 && track_number <= GLFW_KEY_9) {
						int digitSel = track_number - GLFW_KEY_0; // 1..9
						hasSelection = (self && (size_t)digitSel <= self->spriteCount);
					}

					if (hasSelection && action == GLFW_PRESS) {
						key_action = Input_action::reset_transform;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}
				case GLFW_KEY_B:
				{
					if (action == GLFW_PRESS) {
						key_action = Input_action::toggle_collision;
					}
					else {
						key_action = Input_action::no_action;
					}
					break;
				}


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

	else { //store my game mouse input state here
		for (auto& [tuple_mouse, event_action, mouse_action] : mouse_action) {
			if (tuple_mouse == button) {
				switch (tuple_mouse) {
				case GLFW_MOUSE_BUTTON_LEFT:
					if (action == event_action) {
						logger.log("left mouse is pressed");
					}
					break;

				case GLFW_MOUSE_BUTTON_RIGHT:
					if (action == event_action) {
						logger.log("right mouse is pressed");
					}
					break;
				}
			}
		}
	}
}

//void Input::SendMesssage() {
//
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



