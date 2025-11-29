#include "./logic/Animal_Logic.h"

Animal_Logic::Animal_Logic() = default;

std::string Animal_Logic::to_string(const Animal_type& type) {
	switch (type) {
	case Animal_type::Flamingo: return "flamingo";
	case Animal_type::Horse: return "horse";
	case Animal_type::Seal: return "seal";
	case Animal_type::Owl: return "owl";
	default: return "unknown";
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
					//static double acc_timer{ 0 };
					//if (gm.current() != to_string(Animal_type::Flamingo)) {
					//	//acc_timer is to calculate the time player is not viewing it in the cameras
					//    acc_timer += deltaTime;
					//}

					//else {
					//	acc_timer = 0; //reset this timer if player is viewing flamingo currently
					//}

					//if (acc_timer > 3) {
					//	//sanity of flamingo decrease faster when player is not viewing it in the cameras
					//	animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime;
					//	//logger.log("warning: please view the flamingo in the camera");
					//}
					if (gm.current() != to_string(Animal_type::Flamingo)) {
						animal.acc_timer += deltaTime;  
					}
					else {
						animal.acc_timer = 0;  
					}

					if (animal.acc_timer > 3) {  
						animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime;
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
						//static double static_staring_countdown{ 10 };
						//static_staring_countdown -= 0.5;
						//if (static_staring_countdown <= 0) {
						//	//when players look at it for more than x seconds, bar goes down faster
						//	animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime; //sanity of seal decrease faster
						//}
						animal.staring_countdown -= 0.5;  // USE animal.staring_countdown
						if (animal.staring_countdown <= 0) {  //USE animal.staring_countdown
							animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime;
						}
					}
					else {
						// Reset countdown when not staring
						animal.staring_countdown = 10.0f;
					}
				}
			}
		}
	}
}

void Animal_Logic::check_animal_status(GameManager& gm) {
	if (gm.isPlaying()) {
		int deadAnimals = 0;  // Track how many are dead

		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.item_type == "animal")
				{
					// Check if animal should die
					if (animal.health_bar <= 0 || animal.sanity_bar <= 0) {
						if (animal.still_alive) {
							// Animal just died
							animal.still_alive = false;
							logger.log("[Animal] " + animal.name + " has died!");
						}
					}

					// Count dead animals
					if (!animal.still_alive) {
						deadAnimals++;
					}
				}
			}
		}

		// GAME OVER: If 2 or more animals are dead
		if (deadAnimals >= 2) {
			logger.log("[GAME OVER] " + std::to_string(deadAnimals) + " animals have died!");
			gm.triggerGameOver();
		}
	}
}

std::optional<ImVec2> Animal_Logic::check_item_status(
	GameManager& gm, ImVec2 mouse_pos, Input& input, GLFWwindow* window)
{
	if (!gm.isPlaying())
		return std::nullopt;

	Scene* scene = gm.getCurrentScene();
	if (!scene)
		return std::nullopt;

	auto& items = scene->items;

	// one shared offset while dragging
	static float dragX = 0.0f;
	static float dragY = 0.0f;

	for (auto& item : items) {
		if (!item.is_draggable)
			continue;

		// ---- SAME BOUNDING BOX AS EDITOR (top-left + size) ----
		float minX, minY, maxX, maxY;

		if (item.hasCollider && item.colW > 0.0f && item.colH > 0.0f) {
			// collider is top-left (x + offset) with width/height
			minX = item.x + item.colOffsetX;
			minY = item.y + item.colOffsetY;
			maxX = minX + item.colW;
			maxY = minY + item.colH;
		}
		else {
			// sprite rect: top-left (x,y) with width/height
			minX = item.x;
			minY = item.y;
			maxX = item.x + item.w;
			maxY = item.y + item.h;
		}


		bool inside =
			mouse_pos.x >= minX && mouse_pos.x <= maxX &&
			mouse_pos.y >= minY && mouse_pos.y <= maxY;

		// ---- start drag ----
		if (inside && input.isLeftMousePressed(window) && !item.is_dragging) {
			item.is_dragging = true;
			dragX = item.x - mouse_pos.x;
			dragY = item.y - mouse_pos.y;
			return ImVec2{ dragX, dragY };
		}

		// ---- continue drag ----
		if (item.is_dragging && input.isLeftMousePressed(window)) {
			return ImVec2{ dragX, dragY };
		}

		// ---- end drag ----
		if (item.is_dragging && !input.isLeftMousePressed(window)) {
			item.is_dragging = false;
			// don’t early-return; just stop dragging
		}
	}

	return std::nullopt;
}


void Animal_Logic::drag_drop(GameManager& gm,
	ImVec2 mouse_pos,
	Input& input,
	GLFWwindow* window,
	std::optional<ImVec2> drag_value)
{
	auto& item_data{ gm.get_duplicate_item() };

	if (!gm.isPlaying())
		return;

	// If we are dragging a duplicated item, move it with the mouse
	if (item_data && drag_value.has_value()) {
		item_data->x = mouse_pos.x + drag_value.value().x;
		item_data->y = mouse_pos.y + drag_value.value().y;

		// --- centre-based bounds helper (same idea as editor) ---
		auto getBounds = [](const DrawItem& it,
			float& minX, float& minY,
			float& maxX, float& maxY)
			{
				if (it.hasCollider && it.colW > 0.0f && it.colH > 0.0f) {
					// collider: top-left + size
					minX = it.x + it.colOffsetX;
					minY = it.y + it.colOffsetY;
					maxX = minX + it.colW;
					maxY = minY + it.colH;
				}
				else {
					// sprite: top-left + size
					minX = it.x;
					minY = it.y;
					maxX = it.x + it.w;
					maxY = it.y + it.h;
				}
			};


		auto overlapsCenter = [&](const DrawItem& a, const DrawItem& b) {
			float aMinX, aMinY, aMaxX, aMaxY;
			float bMinX, bMinY, bMaxX, bMaxY;

			getBounds(a, aMinX, aMinY, aMaxX, aMaxY);
			getBounds(b, bMinX, bMinY, bMaxX, bMaxY);

			// standard AABB overlap
			bool noOverlap =
				(aMaxX < bMinX) || (aMinX > bMaxX) ||
				(aMaxY < bMinY) || (aMinY > bMaxY);

			return !noOverlap;
			};

		// --- check collision with all animals using centre-based AABB ---
		for (auto& animal : gm.getCurrentScene()->items) {
			if (!item_data) break; // item already consumed

			if (animal.item_type == "animal" && animal.still_alive) {
				if (!overlapsCenter(animal, *item_data))
					continue;

				if (item_data->item_type == "food") {
					animal.hunger_bar += 20.0f;
					if (animal.hunger_bar > 100.0f)
						animal.hunger_bar = 100.0f;
					item_data.reset();
				}
				else if (item_data && item_data->item_type == "medicine") {
					animal.health_bar += 20.0f;
					if (animal.health_bar > 100.0f)
						animal.health_bar = 100.0f;
					item_data.reset();
				}
				else if (item_data && item_data->item_type == "toy") {
					animal.sanity_bar += 20.0f;
					if (animal.sanity_bar > 100.0f)
						animal.sanity_bar = 100.0f;
					item_data.reset();
				}
			}
		}
	}

	// When mouse released, drop/consume item (if still exists)
	if (input.isLeftMouseReleased(window)) {
		item_data.reset(); // releases ownership
	}
}
