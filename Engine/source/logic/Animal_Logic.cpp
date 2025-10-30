#include "./logic/Animal_Logic.h"

Animal_Logic::Animal_Logic() = default;

void Animal_Logic::update_animal_ALLstate() {
	if (stats.current_state == Animal_state::alive) {
		update_animal_health();
		update_animal_hunger();
	}
}

void Animal_Logic::update_animal_health() {
	switch (stats.type) {
	case Animal_type::flamingo:
		if (stats.hunger_bar <= 35) { //health of flamingo decreases faster when hunger bar is low
			stats.health_bar -= 0.1;
		}

		else {
			//health of flamingo decrease slowly when hungry bar is above normal standard
			stats.health_bar -= 0.01;
		}
		break;

	case Animal_type::horse:
		stats.health_bar -= 0.05; //health of horse decrease faster
		break;

	case Animal_type::seal:
		stats.health_bar -= 0.03; //health of seal decrease normally
		break;

	case Animal_type::owl:
		stats.health_bar -= 0.03; //health of owl decrease normally
		break;
	}
}

void Animal_Logic::update_animal_hunger() {
	switch (stats.type) {
	case Animal_type::flamingo:
		stats.hunger_bar -= 0.05; //hunger of flamingo decrease normally
		if (stats.action == Zookeeper_action::feeding){
			if (stats.food == Animal_food::flamingo_food) {
				stats.hunger_bar += 20; //the player feed the flamingo with food
			}

			else if (stats.food == Animal_food::water) {
				stats.hunger_bar += 10; //the player feed the flamingo with water
			}
		}
		break;

	case Animal_type::horse:
		stats.hunger_bar -= 0.05; //health of horse decrease faster
		if (stats.action == Zookeeper_action::feeding) {

		}
		break;

	case Animal_type::seal:
		stats.health_bar -= 0.03; //health of seal decrease normally
		break;

	case Animal_type::owl:
		stats.health_bar -= 0.03; //health of seal decrease normally
		break;
	}
}

Animal_state& Animal_Logic::check_animal_alive() {
	return stats.current_state = stats.health_bar <= 0 ? Animal_state::died : Animal_state::alive;
}

Animal_Logic::animal_stats& Animal_Logic::GET_stat() {
	return stats;
}