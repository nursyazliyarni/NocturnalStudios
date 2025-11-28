#pragma once
#include "core/GameManager.h"
#include "input/Input.h"
#include "graphics/Window.h"
#include "logic/Base_Logic.h"
#include "ECS/ECS.h"

enum class Animal_type {
	Flamingo,
	Horse,
	Seal,
	Owl
};

enum class Animal_state {
	alive,died,escape
};

class Animal_Logic : public Base_Logic  {
public:
	Animal_Logic(ECS& ecs);

	void update(float dt) override; //must be defined by child logic
	void update_animal_ALLstate(GameManager &gm, double deltaTime);

	void update_animal_health(GameManager &gm, double deltaTime);
	void update_animal_hunger(GameManager& gm, double deltaTime);
	void update_animal_sanity(GameManager& gm, double deltaTime);

	void check_animal_status(GameManager& gm);
	std::optional<ImVec2> check_item_status(GameManager& gm, ImVec2 mouse_pos, Input& input, GLFWwindow* window);
	void drag_drop(GameManager& gm, ImVec2 mouse_pos, Input& input, GLFWwindow* window, std::optional<ImVec2> drag_value);

private:
	int accumulation_timer{ 0 };
	ECS& _ecs;
	std::string to_string(const Animal_type& type);
};