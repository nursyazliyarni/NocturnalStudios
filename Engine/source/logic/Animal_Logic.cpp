/*!------------------------------------------------------------------------------------------------
	File: Animal_Logic.cpp
	Project: NocturnalStudio

	This file contains the functions needed for our game logic for our animal game objects

	Primary Author:
		Yu Xuan
		- Implemented update healthbar, hunger bar, sanity bar functions for each animal
		game objects

	Secondary Author:
		Sarag
		-  Modify some of the functions

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "./logic/Animal_Logic.h"


namespace { //helper fns, they shouldnt be put in lambdas?
	// Get bounds using ONLY ECS components

	bool getBounds(EntityID entityID, float& minX, float& minY, float& maxX, float& maxY);
	bool getBounds(EntityID entityID, float& minX, float& minY, float& maxX, float& maxY)
	{
		Transform* t = ECS::getECS().getComponent<Transform>(entityID);
		if (!t) return false;

		// Try box collider first
		if (BoxCollider* bc = ECS::getECS().getComponent<BoxCollider>(entityID)) {
			if (bc->width > 0.0f && bc->height > 0.0f) {
				Vector2D baseSize = { bc->width, bc->height };
				Vector2D scaledSize = { bc->width * t->scale.x(), bc->height * t->scale.y() };
				Vector2D topLeft = t->GetTopLeft(baseSize) + bc->offset;

				minX = topLeft.x();
				minY = topLeft.y();
				maxX = topLeft.x() + scaledSize.x();
				maxY = topLeft.y() + scaledSize.y();
				return true;
			}
		}

		// Try circle collider (convert to bounding box)
		if (CircleCollider* cc = ECS::getECS().getComponent<CircleCollider>(entityID)) {
			if (cc->radius > 0.0f) {
				float diameter = cc->radius * 2.0f;
				Vector2D center = t->GetCenter({ diameter, diameter }) + cc->offset;
				float actualRadius = cc->radius * ((t->scale.x() + t->scale.y()) * 0.5f);

				minX = center.x() - actualRadius;
				minY = center.y() - actualRadius;
				maxX = center.x() + actualRadius;
				maxY = center.y() + actualRadius;
				return true;
			}
		}

		// Fallback to renderer bounds
		if (ImageRenderer* ir = ECS::getECS().getComponent<ImageRenderer>(entityID)) {
			Vector2D baseSize = { ir->width, ir->height };
			Vector2D scaledSize = { ir->width * t->scale.x(), ir->height * t->scale.y() };
			Vector2D topLeft = t->GetTopLeft(baseSize);

			minX = topLeft.x();
			minY = topLeft.y();
			maxX = topLeft.x() + scaledSize.x();
			maxY = topLeft.y() + scaledSize.y();
			return true;
		}

		return false;  // No valid bounds found
	}

	bool overlapsCenter(const SceneItem& a, const SceneItem& b);
	bool overlapsCenter(const SceneItem& a, const SceneItem& b)
	{
		float aMinX, aMinY, aMaxX, aMaxY; float bMinX, bMinY, bMaxX, bMaxY;

		getBounds(a.entityID, aMinX, aMinY, aMaxX, aMaxY);
		getBounds(b.entityID, bMinX, bMinY, bMaxX, bMaxY);

		// standard AABB overlap
		bool noOverlap =
			(aMaxX < bMinX) || (aMinX > bMaxX) ||
			(aMaxY < bMinY) || (aMinY > bMaxY);

		return !noOverlap;
	}
}

Animal_Logic::Animal_Logic() = default;

void Animal_Logic::update(float dt) {

}

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
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				AnimalBehavior* animalb = ECS::getECS().getComponent<AnimalBehavior>(animal.entityID);
				if (!animalb) continue;
				if (animal.name == to_string(Animal_type::Flamingo) && animalb->still_alive) {
					if (animalb->hunger_bar <= animalb->hunger_bar / 3) { //health of flamingo decreases faster when hunger bar is low
						animalb->health_bar -= animalb->penalty_decreasing_rate * deltaTime;
					}

					else {
						//health of flamingo decrease slowly when hungry bar is above normal standard
						animalb->health_bar -= animalb->slower_decreasing_rate * deltaTime;
					}
				}

				else if (animal.name == to_string(Animal_type::Horse) && animalb->still_alive) {
					animalb->health_bar -= animalb->faster_decreasing_rate * deltaTime; //health of horse decrease faster
				}

				else if (animalb->still_alive && (animal.name == to_string(Animal_type::Seal) || animal.name == to_string(Animal_type::Owl))) {
					animalb->health_bar -= animalb->normal_decreasing_rate * deltaTime; //health of seal/owl decrease normally
				}
			}
		}
	}
}

void Animal_Logic::update_animal_hunger(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				AnimalBehavior* animalb = ECS::getECS().getComponent<AnimalBehavior>(animal.entityID);
				if (!animalb) continue;
				if (animalb->still_alive && (animal.name == to_string(Animal_type::Flamingo) || animal.name == to_string(Animal_type::Owl))) {
					animalb->hunger_bar -= animalb->normal_decreasing_rate * deltaTime; //hunger of flamingo/owl decrease normally
				}

				else if (animalb->still_alive && animal.name == to_string(Animal_type::Horse)) {
					animalb->hunger_bar -= animalb->normal_decreasing_rate * deltaTime; //hunger of horse decrease normally
					/*static bool is_horseHAPPY{ true };
					if (accumulation_timer > 30) {
						is_horseHAPPY = false;
					}*/
				}

				else if (animalb->still_alive && animal.name == to_string(Animal_type::Seal)) {
					animalb->hunger_bar -= animalb->normal_decreasing_rate * deltaTime; //hunger of seal decrease normally
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
				AnimalBehavior* animalb = ECS::getECS().getComponent<AnimalBehavior>(animal.entityID);
				if (!animalb) continue;
				if (animalb->still_alive && animal.name == to_string(Animal_type::Flamingo)) {
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
						animalb->acc_timer += deltaTime;
					}
					else {
						animalb->acc_timer = 0;
					}

					if (animalb->acc_timer > 3) {
						animalb->sanity_bar -= animalb->penalty_decreasing_rate * deltaTime;
					}

					else {
						//sanity of flamingo decrease normally
						animalb->sanity_bar -= animalb->normal_decreasing_rate * deltaTime;
					}
				}

				else if (animalb->still_alive && animal.name == to_string(Animal_type::Horse)) {
					animalb->sanity_bar -= animalb->faster_decreasing_rate * deltaTime; //sanity of horse decrease faster
				}

				else if (animalb->still_alive && animal.name == to_string(Animal_type::Owl)) {
					animalb->sanity_bar -= animalb->normal_decreasing_rate * deltaTime; //sanity of owl decrease normally
				}

				else if (animalb->still_alive && animal.name == to_string(Animal_type::Seal)) {
					animalb->sanity_bar -= animalb->normal_decreasing_rate * deltaTime; //sanity of seal decrease normally
					//if (stats.action == Zookeeper_action::staring) {
					//	static double static_staring_countdown{ 10 };
					//	static_staring_countdown -= 0.5;
					//	if (static_staring_countdown <= 0) {
					//		//when players look at it for more than x seconds, bar goes down faster
					//		animal.sanity_bar -= animal.penalty_decreasing_rate * deltaTime; //sanity of seal decrease faster
					//	}
					//}
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
				AnimalBehavior* animalb = ECS::getECS().getComponent<AnimalBehavior>(animal.entityID);
				if (!animalb) continue;
				if (animal.item_type == "animal")
				{
					// Check if animal should die
					if (animalb->health_bar <= 0 || animalb->sanity_bar <= 0) {
						if (animalb->still_alive) {
							// Animal just died
							animalb->still_alive = false;
							logger.log("[Animal] " + animal.name + " has died!");
						}
					}

					// Count dead animals
					if (!animalb->still_alive) {
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

		Draggable* d = ECS::getECS().getComponent<Draggable>(item.entityID);

		if (d == nullptr || item.item_type == "animal")
			continue;

		bool inside{ false };
		BoxCollider* bc = ECS::getECS().getComponent<BoxCollider>(item.entityID);
		CircleCollider* cc = ECS::getECS().getComponent<CircleCollider>(item.entityID);

		Transform* t = ECS::getECS().getComponent<Transform>(item.entityID);

		if (!t || (!bc && !cc)) continue;

		if (bc)
		{
			// ---- SAME BOUNDING BOX AS EDITOR (top-left + size) ----
			float minX{}, minY{}, maxX{}, maxY{};
			ImageRenderer* ir = ECS::getECS().getComponent<ImageRenderer>(item.entityID);

			Vector2D topLeft{};

			if (bc->width > 0.0f && bc->height > 0.0f && t) {
				topLeft = t->GetTopLeft({ bc->width,bc->height });
				// collider is top-left (x + offset) with width/height
				minX = topLeft.x() + bc->offset.x();
				minY = topLeft.y() + bc->offset.y();
				maxX = minX + (bc->width * t->scale.x());
				maxY = minY + (bc->height * t->scale.y());
			}
			else if (t && ir) {
				topLeft = t->GetTopLeft({ ir->width,ir->height });
				// sprite rect: top-left (x,y) with width/height
				minX = topLeft.x();
				minY = topLeft.y();
				maxX = topLeft.x() + (ir->width * t->scale.x());
				maxY = topLeft.y() + (ir->height * t->scale.y());
			}


			inside =
				mouse_pos.x >= minX && mouse_pos.x <= maxX &&
				mouse_pos.y >= minY && mouse_pos.y <= maxY;
		}
		else if (cc) {

			if (cc->radius > 0.f)
			{
				// Get circle center in world space
				float diameter = cc->radius * 2.0f;
				Vector2D center = t->GetCenter({ diameter, diameter });

				center += cc->offset;

				// Get actual scaled radius
				float actualRadius = cc->radius * ((t->scale.x() + t->scale.y()) * 0.5f);

				// Distance squared check (faster - avoids sqrt)
				Vector2D diff = Vector2D{ mouse_pos.x,mouse_pos.y } - center;
				float distSq = diff.x() * diff.x() + diff.y() * diff.y();
				float radiusSq = actualRadius * actualRadius;

				inside = distSq <= radiusSq;
			}
		}


		// ---- start drag ----
		if (inside && input.isLeftMousePressed(window) && !d->is_dragging && t) {
			d->is_dragging = true;
			dragX = t->position.x() - mouse_pos.x;
			dragY = t->position.y() - mouse_pos.y;
			return ImVec2{ dragX, dragY };
		}

		// ---- continue drag ----
		if (d->is_dragging && input.isLeftMousePressed(window)) {
			return ImVec2{ dragX, dragY };
		}

		// ---- end drag ----
		if (d->is_dragging && !input.isLeftMousePressed(window)) {
			d->is_dragging = false;
			// don�t early-return; just stop dragging
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
		Transform* t = ECS::getECS().getComponent<Transform>(item_data->entityID);
		t->position.x() = mouse_pos.x + drag_value.value().x;
		t->position.y() = mouse_pos.y + drag_value.value().y;

		// --- check collision with all animals using centre-based AABB ---
		for (auto& animal : gm.getCurrentScene()->items) {
			AnimalBehavior* animalb = ECS::getECS().getComponent<AnimalBehavior>(animal.entityID);
			if (!animalb) continue;
			if (!item_data) break; // item already consumed

			if (animal.item_type == "animal" && animalb->still_alive) {
				if (!overlapsCenter(animal, *item_data))
					continue;

				if (item_data->item_type == "food") {
					animalb->hunger_bar += 20.0f;
					if (animalb->hunger_bar > 100.0f)
						animalb->hunger_bar = 100.0f;
					if (item_data)item_data->DestroyDupe();
					item_data.reset();
				}
				else if (item_data && item_data->item_type == "medicine") {
					animalb->health_bar += 20.0f;
					if (animalb->health_bar > 100.0f)
						animalb->health_bar = 100.0f;
					if (item_data)item_data->DestroyDupe();
					item_data.reset();
				}
				else if (item_data && item_data->item_type == "toy") {
					animalb->sanity_bar += 20.0f;
					if (animalb->sanity_bar > 100.0f)
						animalb->sanity_bar = 100.0f;
					if (item_data)item_data->DestroyDupe();
					item_data.reset();
				}
			}
		}
	}

	// When mouse released, drop/consume item (if still exists)
	if (input.isLeftMouseReleased(window)) {
		if (item_data)item_data->DestroyDupe();
		item_data.reset(); // releases ownership
	}
}
