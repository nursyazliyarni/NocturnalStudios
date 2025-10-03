/*!------------------------------------------------------------------------------------------------
    File: GameLoop.h
    Project: NocturnalStudio

    This file contains functions that calculates deltaTime which is needed for physics,
    accumulated game time and FPS when the game is running.

    Primary Author:
         Ang Yu Xuan (80%)
        -Implement the function to calculate deltaTime, accumulated game time and FPS

    Secondary Author:
         Nur Syazliyarni (20%)
        -Added  the messaging system as part of game loop

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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