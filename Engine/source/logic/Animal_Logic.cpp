#include "./logic/Animal_Logic.h"

Animal_Logic::Animal_Logic() = default;

void Animal_Logic::update_animal_ALLstate(GameManager& gm) {
	if (accumulation_timer >= 60) {
		accumulation_timer = 0;
	}
	accumulation_timer += 1;

	//if (stats.current_state == Animal_state::alive) {
		update_animal_health(gm);
		//update_animal_hunger();
		//update_animal_sanity();
	//}
}

void Animal_Logic::update_animal_health(GameManager& gm) {
	for (auto& animal : gm.getCurrentScene()->items) {
		if (animal.name == "flamingo") {
			if (animal.health_bar <= 35) { //health of flamingo decreases faster when hunger bar is low
				animal.health_bar -= 0.1;
			}

			else {
				//health of flamingo decrease slowly when hungry bar is above normal standard
				animal.health_bar -= 0.01;
			}
		}

		else if (animal.name == "horse") {
			animal.health_bar -= 0.05; //health of horse decrease faster
		}

		else if (animal.name == "seal" || animal.name == "owl") {
			animal.health_bar -= 0.03; //health of seal/owl decrease normally
		}

		std::cout << animal.name << " " << animal.health_bar << "\n";
	}
	std::cout << "Logic is running\n";
}

//void Animal_Logic::update_animal_hunger() {
//	switch (stats.type) {
//	case Animal_type::flamingo:
//		stats.hunger_bar -= 0.05; //hunger of flamingo decrease normally
//		if (stats.action == Zookeeper_action::feeding && stats.food == Animal_food::flamingo_food) {
//			stats.hunger_bar += 20; //the player feed the flamingo with food
//		}
//		break;
//
//	case Animal_type::horse:
//		stats.hunger_bar -= 0.05; //hunger of horse decrease normally
//		if (stats.action == Zookeeper_action::feeding && stats.food == Animal_food::horse_food) {
//			static bool is_horseHAPPY{ true };
//			if (accumulation_timer > 30) {
//				is_horseHAPPY = false;
//			}
//			stats.hunger_bar += 20; //the player feed the horse with food
//		}
//		break;
//
//	case Animal_type::seal:
//		stats.hunger_bar -= 0.05; //hunger of seal decrease normally
//		if (stats.action == Zookeeper_action::feeding && stats.food == Animal_food::seal_food) {
//			static bool is_sealHAPPY{ true };
//			if (accumulation_timer <= 15 && accumulation_timer >= 45) {
//				is_sealHAPPY = false;
//			}
//			stats.hunger_bar += 20; //the player feed the seal with food
//		}
//		break;
//
//	case Animal_type::owl:
//		stats.hunger_bar -= 0.05; //hunger of owl decrease normally
//		if (stats.action == Zookeeper_action::feeding && stats.food == Animal_food::owl_food) {
//			stats.hunger_bar += 20; //the player feed the owl with food
//		}
//		break;
//	}
//}
//
//void Animal_Logic::update_animal_sanity() {
//	switch (stats.type) {
//	case Animal_type::flamingo:
//		if (stats.action == Zookeeper_action::staring) {
//			stats.sanity_bar -= 0.05; //sanity of flamingo decrease normally
//		}
//
//		else { //sanity of flamingo decrease faster when player is not viewing it in the cameras
//			stats.sanity_bar -= 0.1;
//		}
//		break;
//
//	case Animal_type::horse:
//		stats.sanity_bar -= 0.1; //sanity of horse decrease faster
//		break;
//
//	case Animal_type::seal:
//		stats.sanity_bar -= 0.05; //sanity of seal decrease normally
//		if (stats.action == Zookeeper_action::staring) {
//			static double static_staring_countdown{ 10 };
//			static_staring_countdown -= 0.5;
//			if (static_staring_countdown <= 0) {
//				//when players look at it for more than x seconds, bar goes down faster
//				stats.sanity_bar -= 0.1; //sanity of seal decrease faster
//			}
//		}
//		break;
//
//	case Animal_type::owl:
//		stats.sanity_bar -= 0.05; //sanity of owl decrease normally
//		break;
//	}
//}
//
//Animal_state& Animal_Logic::check_animal_status() {
//	return stats.current_state = (stats.health_bar <= 0) ? Animal_state::died : 
//		   (stats.sanity_bar <= 0) ? Animal_state::escape: Animal_state::alive;
//}

Animal_Logic::animal_stats& Animal_Logic::GET_stat() {
	return stats;
}