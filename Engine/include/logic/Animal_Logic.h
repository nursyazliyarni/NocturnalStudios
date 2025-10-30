#pragma once

enum class Animal_food {
	none,flamingo_food,horse_food,seal_food,owl_food
};

enum class Animal_mood {
	normal,happy,sad
};

enum class Animal_type {
	flamingo,horse,seal,owl
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

	void update_animal_ALLstate();

	void update_animal_health();
	void update_animal_hunger();
	void update_animal_sanity();

	Animal_state& check_animal_status();

private:
	struct animal_stats {
		bool IS_active;
		Animal_type type;
		Zookeeper_action action;
		Animal_food food;

		Animal_state current_state;
		double health_bar, hunger_bar, sanity_bar;
	}stats;
	int accumulation_timer{ 0 };

	animal_stats& GET_stat();
};