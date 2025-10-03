// delta, fixedStep, FPS
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include "messaging/Message.h"
#include "messaging/MessageHandler.h"

class Game_Loop : public Messaging::IMessageHandler {
public:
	Game_Loop(GLFWwindow* window);
	double calculate_DT();
	double get_accum_GT();
	void display_FPS(GLFWwindow* window, std::string game_title, double deltaTime);

	void HandleMessage(Messaging::Message* msg) override; 

private:
	double accum_game_time; //total accumulated game time
	double curr_frame_time; //to measure the time at the start of each frame
	double display_interval;
};

//class Game_Loop : public Messaging::IMessageHandler {
//public:
//	void HandleMessage(Messaging::Message* msg) override;
//};