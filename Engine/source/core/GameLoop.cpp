/*!------------------------------------------------------------------------------------------------
    File: GameLoop.cpp
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
        -Brief

    Secondary Author:
         Name (%)
        -Brief

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// delta, fixedStep, FPS
#include "core/GameLoop.h"
#include "messaging/Message.h"
#include "messaging/InputMessage.h"
#include "messaging/MessageHandler.h"

Game_Loop::Game_Loop(GLFWwindow* window) : accum_game_time{ 0.0 }, curr_frame_time{ glfwGetTime() }, 
display_interval{ 0.0 } {}

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

//messaging system implemented in game loop
void Game_Loop::HandleMessage(Messaging::Message* msg) {
    switch (msg->MessageId) {
    case Messaging::Mid::CHARACTERKEY: {
        auto* keyMsg = static_cast<Messaging::KeyMessage*>(msg);
        if (keyMsg->action == GLFW_PRESS) {
            std::cout << "[GameLoop] Key pressed: " << keyMsg->keyCode << "\n";
        }
        else if (keyMsg->action == GLFW_RELEASE) {
            std::cout << "[GameLoop] Key released: " << keyMsg->keyCode << "\n";
        }
        break;
    }
    /*case Messaging::Mid::MOUSEMOVE: {
        auto* mouseMove = static_cast<Messaging::MovingMouseMessage*>(msg);
        std::cout << "[GameLoop] Mouse moved to (" << mouseMove->x << ", " << mouseMove->y << ")\n";
        break;
    }*/
    case Messaging::Mid::MOUSEBUTTON: {
        auto* mouseBtn = static_cast<Messaging::ClickedMouseMessage*>(msg);
        std::cout << "[GameLoop] Mouse button " << mouseBtn->MouseButtonIndex
            << (mouseBtn->buttonPressed ? " pressed" : " released")
            << " at (" << mouseBtn->mouseX << ", " << mouseBtn->mouseY << ")\n";
        break;
    }
    default: break;
    }
}
