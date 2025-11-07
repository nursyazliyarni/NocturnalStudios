#pragma once
#include "core/GameManager.h"

enum class Animal_food {
	none,water,flamingo_food,horse_food,seal_food,owl_food
};

enum class Animal_mood {
	normal,happy,sad
};

enum class Animal_state {
	alive,died,escape
};

enum class Zookeeper_action {
	none,feeding,staring
};

class Animal_Logic {
public:
	Animal_Logic();

	void update_animal_ALLstate(GameManager &gm);

	void update_animal_health(GameManager &gm);
	void update_animal_hunger();
	void update_animal_sanity();

	Animal_state& check_animal_status();

private:
	struct animal_stats {
		bool IS_active;
		Zookeeper_action action;
		Animal_food food;

		Animal_state current_state;
	}stats;
	int accumulation_timer{ 0 };

	animal_stats& GET_stat();
};