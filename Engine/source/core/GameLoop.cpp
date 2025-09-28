// delta, fixedStep, FPS
#include "core/GameLoop.h"

Game_Loop::Game_Loop(GLFWwindow* window) : accum_game_time{ 0.0 }, curr_frame_time{ glfwGetTime() }, 
display_interval{ 0.0 }, game_loop_input{window} {}

double Game_Loop::calculate_DT() {
	double new_frame_time = glfwGetTime();
	double DT{ new_frame_time - curr_frame_time };
	curr_frame_time = new_frame_time;
	accum_game_time += DT; //accumulate total game time
	display_interval += DT;

	return DT;
}

double Game_Loop::get_accum_GT() {
	return accum_game_time;
}

void Game_Loop::display_FPS(GLFWwindow *window, std::string game_title, double deltaTime) {
	std::stringstream ss;
	ss << game_title << " | Our game current FPS : " << 1 / deltaTime;
	std::string window_title = ss.str();

	if (display_interval >= 1.0) {
		glfwSetWindowTitle(window, window_title.c_str());
		display_interval = 0.0;
	}
}

void Game_Loop::update(GLFWwindow* window) {
	//update sequence->Inputs, Logic, Physics, Collision, Audio, Graphics
	//game_loop_input.process_user_input(window); //game loop take charge of game input
	//update logic
	//update physics
	//update collision
	//update audio
	//update graphics
}
