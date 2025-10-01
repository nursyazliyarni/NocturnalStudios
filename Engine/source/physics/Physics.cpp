// integration, gravity, movement
#include "input/Input.h"
#include "physics/Physics.h"

int Physics::take_control(Renderer& render_obj, Input& input_obj) {
	auto& keyboard_action = input_obj.GET_keyboard();
	for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
		if (key_action == Input_action::take_control) {
			return tuple_key - GLFW_KEY_1;
		}
	}
	return -1;
}

void Physics::update_sprite_movement(Renderer& render_obj, Input& input_obj){
	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

	if (index >= 0) {
		auto& keyboard_action = input_obj.GET_keyboard();
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::move_up) {
				sprite_vector[index].y -= 1.0f;
			}

			if (key_action == Input_action::move_down) {
				sprite_vector[index].y += 1.0f;
			}

			if (key_action == Input_action::move_left) {
				sprite_vector[index].x -= 1.0f;
			}

			if (key_action == Input_action::move_right) {
				sprite_vector[index].x += 1.0f;
			}
		}
	}
}
