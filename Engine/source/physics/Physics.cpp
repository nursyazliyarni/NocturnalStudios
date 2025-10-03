// integration, gravity, movement
#include "input/Input.h"
#include "physics/Physics.h"
#include "physics/AABB.h"

int Physics::take_control(Renderer& render_obj, Input& input_obj) {
	auto& keyboard_action = input_obj.GET_keyboard();
	for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
		if (key_action == Input_action::take_control) {
			return tuple_key - GLFW_KEY_1;
		}
	}
	return -1; //return invalid index here
}

void Physics::update_sprite_movement(Renderer& render_obj, Input& input_obj){
	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

	if (index >= 0) {
		auto& sprite = sprite_vector[index]; //get sprite to move
		auto& keyboard_action = input_obj.GET_keyboard();
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::move_up) {
				sprite_vector[index].y -= 0.001f;
			}

			else if (key_action == Input_action::move_down) {
				sprite_vector[index].y += 0.001f;
			}

			else if (key_action == Input_action::move_left) {
				sprite_vector[index].x -= 0.001f;
			}

			else if (key_action == Input_action::move_right) {
				sprite_vector[index].x += 0.001f;
			}
		}
		//collision detection with other sprites through movement and AABB
		for (size_t i = 0; i < sprite_vector.size(); i++) { //ignore background at index 0
			if (i == index) continue; //skip the controlled sprite
			const auto& other = sprite_vector[i];

			Vector2D pos1(sprite.x, sprite.y);
			Vector2D size1(sprite.w, sprite.h);

			Vector2D pos2(other.x, other.y);
			Vector2D size2(other.w, other.h);

			if (checkAABBCollision(pos1, size1, pos2, size2)) {
				std::cout << "Collision detected with sprite " << i+1 << std::endl;

				// Optional: Handle the collision (undo movement, bounce back, etc.)
			}
		}
	}
}

void Physics::update_sprite_scaling(Renderer& render_obj, Input& input_obj) {
	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

	if (index >= 0) {
		auto& keyboard_action = input_obj.GET_keyboard();
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::magnify) {
				sprite_vector[index].w += 0.001f, sprite_vector[index].h += 0.001f;
			}

			else if (key_action == Input_action::minify) {
				sprite_vector[index].w -= 0.001f, sprite_vector[index].h -= 0.001f;
			}
		}
	}
}

void Physics::update_sprite_rotation(Renderer& render_obj, Input& input_obj) {
	int index = take_control(render_obj, input_obj);
	auto& sprite_vector = render_obj.get_sprite_vector();

	if (index >= 0) {
		auto& keyboard_action = input_obj.GET_keyboard();
		for (auto& [tuple_key, event_action, key_action] : keyboard_action) {
			if (key_action == Input_action::rotate_clockwise) {
				sprite_vector[index].angle -= 0.1f;
			}

			else if (key_action == Input_action::rotate_anticlockwise) {
				sprite_vector[index].angle += 0.1f;
			}
		}
	}
}