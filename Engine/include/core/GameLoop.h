// delta, fixedStep, FPS
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include "input/Input.h"

class Game_Loop {
public:
	Game_Loop(GLFWwindow* window);
	double calculate_DT();
	double get_accum_GT();
	void display_FPS(GLFWwindow* window, std::string game_title, double deltaTime);

	void update(GLFWwindow* window);

private:
	double accum_game_time; //total accumulated game time
	double curr_frame_time; //to measure the time at the start of each frame
	double display_interval;

	//Game Loop takes charge of all the componets listed here-> has-a relationship(composition)
	//Game Loop components here
	Input game_loop_input;
	//Logic 
	//Physics
	//Collision
	//Audio
	//Graphics
};