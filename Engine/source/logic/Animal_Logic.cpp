#include "./logic/Animal_Logic.h"

Animal_Logic::Animal_Logic() = default;

std::string Animal_Logic::to_string(const Animal_type& type) {
	switch (type) {
	case Animal_type::Flamingo: return "flamingo";
	case Animal_type::Horse: return "horse";
	case Animal_type::Seal: return "seal";
	case Animal_type::Owl: return "owl";
	}
}

void Animal_Logic::update_animal_ALLstate(GameManager& gm, double deltaTime) {
	update_animal_health(gm, deltaTime);
	update_animal_hunger(gm, deltaTime);
	update_animal_sanity(gm, deltaTime);
}

void Animal_Logic::update_animal_health(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()){
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.name == to_string(Animal_type::Flamingo) && animal.still_alive) {
					if (animal.hunger_bar <= animal.hunger_bar/3) { //health of flamingo decreases faster when hunger bar is low
						animal.health_bar -= animal.penalty_decreasing_rate * deltaTime;
					}

					else {
						//health of flamingo decrease slowly when hungry bar is above normal standard
						animal.health_bar -= animal.slower_decreasing_rate * deltaTime;
					}
				}

				else if (animal.name == to_string(Animal_type::Horse) && animal.still_alive) {
					animal.health_bar -= animal.faster_decreasing_rate * deltaTime; //health of horse decrease faster
				}

				else if (animal.still_alive && (animal.name == to_string(Animal_type::Seal) || animal.name == to_string(Animal_type::Owl))) {
					animal.health_bar -= animal.normal_decreasing_rate * deltaTime; //health of seal/owl decrease normally
				}
			}
		}
	}
}

void Animal_Logic::update_animal_hunger(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.still_alive && (animal.name == to_string(Animal_type::Flamingo) || animal.name == to_string(Animal_type::Owl))) {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of flamingo/owl decrease normally
				}

				else if (animal.still_alive && animal.name == to_string(Animal_type::Horse)) {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of horse decrease normally
					/*static bool is_horseHAPPY{ true };
					if (accumulation_timer > 30) {
						is_horseHAPPY = false;
					}*/
				}

				else if (animal.still_alive && animal.name == to_string(Animal_type::Seal)) {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of seal decrease normally
					//if (stats.action == Zookeeper_action::feeding) {
					//	static bool is_sealHAPPY{ true };
					//	if (accumulation_timer <= 15 && accumulation_timer >= 45) {
					//		is_sealHAPPY = false;
					//	}
					//	//animal.hunger_bar += 20; //the player feed the seal with food
					//}
				}
			}
		}
	}
}

void Animal_Logic::update_animal_sanity(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.still_alive && animal.name == to_string(Animal_type::Flamingo)) {
					static double acc_timer{ 0 };
					if (gm.current() != to_string(Animal_type::Flamingo)) {
						//acc_timer is to calculate the time player is not viewing it in the cameras
					    acc_timer += deltaTime;
					}

					else {
						acc_timer = 0; //reset this timer if player is viewing flamingo currently
					}

					if (acc_timer > 3) {
						//sanity of flamingo decrease faster when player is not viewing it in the cameras
						animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime;
						//logger.log("warning: please view the flamingo in the camera");
					}

					else {
						//sanity of flamingo decrease normally
						animal.sanity_bar -= animal.normal_decreasing_rate * deltaTime;
					}
				}

				else if (animal.still_alive && animal.name == to_string(Animal_type::Horse)) {
					animal.sanity_bar -= animal.faster_decreasing_rate * deltaTime; //sanity of horse decrease faster
				}

				else if (animal.still_alive && animal.name == to_string(Animal_type::Owl)) {
					animal.sanity_bar -= animal.normal_decreasing_rate * deltaTime; //sanity of owl decrease normally
				}

				else if (animal.still_alive && animal.name == to_string(Animal_type::Seal)) {
					animal.sanity_bar -= animal.normal_decreasing_rate * deltaTime; //sanity of seal decrease normally
					if (stats.action == Zookeeper_action::staring) {
						static double static_staring_countdown{ 10 };
						static_staring_countdown -= 0.5;
						if (static_staring_countdown <= 0) {
							//when players look at it for more than x seconds, bar goes down faster
							animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime; //sanity of seal decrease faster
						}
					}
				}
			}
		}
	}
}

void Animal_Logic::check_animal_status(GameManager& gm) {
	if (gm.isPlaying()){
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.item_type == "animal")
				{
					if (animal.health_bar <= 0 || animal.sanity_bar <= 0) {
						animal.still_alive = false;
					}
				}
			}
		}
	}
}

std::optional<ImVec2> Animal_Logic::check_item_status(GameManager& gm, ImVec2 mouse_pos, Input& input, GLFWwindow* window) {
	if (gm.isPlaying()) {
		for (auto& item : gm.getCurrentScene()->items) {
			if (item.is_draggable) {
				static float dragX = 0.0f;
				static float dragY = 0.0f;
				if ((mouse_pos.x >= item.x) && (mouse_pos.x <= item.x + item.w)
					&& mouse_pos.y >= item.y && mouse_pos.y <= item.y + item.h
					&& input.isLeftMousePressed(window) && !item.is_dragging)
				{
					item.is_dragging = true;
					dragX = item.x - mouse_pos.x;
					dragY = item.y - mouse_pos.y;
					return ImVec2{ dragX, dragY }; //return value
				}

				// The player is still dragging the item
				if (item.is_dragging && input.isLeftMousePressed(window)) {
					return ImVec2{ dragX, dragY };
				}

				if (!input.isLeftMousePressed(window) && item.is_dragging) {
					item.is_dragging = false;
					return std::nullopt; //return no value
				}
			}
		}
	}
	return std::nullopt; //return no value
}

void Animal_Logic::drag_drop(GameManager& gm, ImVec2 mouse_pos, Input& input, GLFWwindow* window, std::optional<ImVec2> drag_value)
{
	auto& item_data{ gm.get_duplicate_item() };
	static bool feeding{ false };
	if (gm.isPlaying()) { 
		if (item_data && drag_value.has_value() && !feeding) {
			item_data->x = mouse_pos.x + drag_value.value().x;
			item_data->y = mouse_pos.y + drag_value.value().y;
			for (auto& animal : gm.getCurrentScene()->items) {
				if (animal.item_type == "animal" && animal.still_alive) {
					if (checkCollision(animal.x, animal.y, animal.w, animal.h, item_data->x, item_data->y, item_data->w, item_data->h)
						&&item_data->item_type == "food")
					{
						feeding = true;
						animal.hunger_bar += 20;
						if (animal.hunger_bar >= 100) {
							animal.hunger_bar = 100;
						}
						item_data.reset(); //The unique_ptr releases ownership
					}

					else if (checkCollision(animal.x, animal.y, animal.w, animal.h, item_data->x, item_data->y, item_data->w, item_data->h)
						&& item_data->item_type == "medicine")
					{
						feeding = true;
						animal.health_bar += 20;
						if (animal.health_bar >= 100) {
							animal.health_bar = 100;
						}
						item_data.reset(); //The unique_ptr releases ownership
					}

					else if (checkCollision(animal.x, animal.y, animal.w, animal.h, item_data->x, item_data->y, item_data->w, item_data->h)
						&& item_data->item_type == "toy")
					{
						feeding = true;
						animal.sanity_bar += 20;
						if (animal.sanity_bar >= 100) {
							animal.sanity_bar = 100;
						}
						item_data.reset(); //The unique_ptr releases ownership
					}
				}
			}
		}

		if (input.isLeftMouseReleased(window)) {
			feeding = false;
			item_data.reset(); //The unique_ptr releases ownership
		}
	}
}