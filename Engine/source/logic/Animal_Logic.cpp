#include "./logic/Animal_Logic.h"

Animal_Logic::Animal_Logic() = default;

void Animal_Logic::update_animal_ALLstate(GameManager& gm, double deltaTime) {
	update_animal_health(gm, deltaTime);
	update_animal_hunger(gm, deltaTime);
	update_animal_sanity(gm, deltaTime);
}

void Animal_Logic::update_animal_health(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()){
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.name == "flamingo") {
					if (animal.hunger_bar <= 35) { //health of flamingo decreases faster when hunger bar is low
						animal.health_bar -= animal.penalty_decreasing_rate * deltaTime;
					}

					else {
						//health of flamingo decrease slowly when hungry bar is above normal standard
						animal.health_bar -= animal.slower_decreasing_rate * deltaTime;
					}
				}

				else if (animal.name == "horse") {
					animal.health_bar -= animal.faster_decreasing_rate * deltaTime; //health of horse decrease faster
					//std::cout << animal.name << " " << animal.health_bar << "\n";
				}

				else if (animal.name == "seal" || animal.name == "owl") {
					animal.health_bar -= animal.normal_decreasing_rate * deltaTime; //health of seal/owl decrease normally
					//std::cout << animal.name << " " << animal.health_bar << "\n";
				}
			}
		}
	}
}

void Animal_Logic::update_animal_hunger(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.name == "flamingo" || animal.name == "owl") {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of flamingo/owl decrease normally
					if (stats.action == Zookeeper_action::feeding) {
						animal.hunger_bar += 20; //the player feed the flamingo/owl with food
					}
				}

				else if (animal.name == "horse") {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of horse decrease normally
					static bool is_horseHAPPY{ true };
					if (accumulation_timer > 30) {
						is_horseHAPPY = false;
					}
					//animal.hunger_bar += 20; //the player feed the horse with food

				}

				else if (animal.name == "seal") {
					animal.hunger_bar -= animal.normal_decreasing_rate * deltaTime; //hunger of seal decrease normally
					if (stats.action == Zookeeper_action::feeding) {
						static bool is_sealHAPPY{ true };
						if (accumulation_timer <= 15 && accumulation_timer >= 45) {
							is_sealHAPPY = false;
						}
						animal.hunger_bar += 20; //the player feed the seal with food
					}
				}
			}
		}
	}
}

void Animal_Logic::update_animal_sanity(GameManager& gm, double deltaTime) {
	if (gm.isPlaying()) {
		for (auto& [scene, scene_data] : gm.get_scenes_map()) {
			for (auto& animal : scene_data.items) {
				if (animal.name == "flamingo") {
					static double acc_timer{ 0 };
					if (gm.current() != "Flamingo") {
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
						glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
					}

					else {
						//sanity of flamingo decrease normally
						animal.sanity_bar -= animal.normal_decreasing_rate * deltaTime;
					}
				}

				else if (animal.name == "horse") {
					animal.sanity_bar -= animal.faster_decreasing_rate * deltaTime; //sanity of horse decrease faster
				}

				else if (animal.name == "owl") {
					animal.sanity_bar -= animal.normal_decreasing_rate * deltaTime; //sanity of owl decrease normally
				}

				else if (animal.name == "seal") {
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
	for (auto& [scene, scene_data] : gm.get_scenes_map()) {
		for (auto& animal : scene_data.items) {
			if (animal.name == "flamingo" || animal.name == "horse" || animal.name == "owl" || animal.name == "seal")
			{
				if (animal.health_bar <= 0 || animal.sanity_bar <= 0) {
					animal.still_alive = false;
				}
			}
		}
	}
}