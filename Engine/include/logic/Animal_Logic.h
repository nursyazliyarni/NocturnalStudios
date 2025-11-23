#pragma once
#include "core/GameManager.h"

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

	void update_animal_ALLstate(GameManager &gm, double deltaTime);

	void update_animal_health(GameManager &gm, double deltaTime);
	void update_animal_hunger(GameManager& gm, double deltaTime);
	void update_animal_sanity(GameManager& gm, double deltaTime);

	void check_animal_status(GameManager& gm);

private:
	struct animal_stats {
		bool IS_active;
		Zookeeper_action action;

		Animal_state current_state;
	}stats;
	int accumulation_timer{ 0 };
};