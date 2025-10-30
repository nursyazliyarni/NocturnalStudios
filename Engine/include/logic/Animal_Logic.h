#pragma once

enum class Animal_food {
	none,flamingo_food,horse_food,seal_food,owl_food
};

enum class Animal_type {
	flamingo,horse,seal,owl
};

enum class Animal_state {
	alive,died
};

enum class Zookeeper_action {
	none,feeding,
};

class Animal_Logic {
public:
	Animal_Logic();

	void update_animal_ALLstate();

	void update_animal_health();
	void update_animal_hunger();

	Animal_state& check_animal_alive();

private:
	struct animal_stats {
		bool IS_active;
		Animal_type type;
		Zookeeper_action action;
		Animal_food food;

		Animal_state current_state;
		double health_bar, hunger_bar, sanity_bar;
	}stats;

	animal_stats& GET_stat();
};