/*!------------------------------------------------------------------------------------------------
    File: GameLoop.cpp
    Project: NocturnalStudio

    This file contains ...

    Primary Author:
         Name (%)
        -Brief

    Secondary Author:
         Nur Syazliyarni 20%
        - Added the messaging system

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

// delta, fixedStep, FPS
#include "core/GameLoop.h"
#include "messaging/Message.h"
#include "messaging/InputMessage.h"
#include "messaging/MessageHandler.h"
#include "graphics/DebugDraw.h"

static bool debugToggled = false;

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

            // Only toggle when the key is B AND not already toggled in this press
            if (keyMsg->keyCode == GLFW_KEY_B && !debugToggled) {
                auto& drawer = Debug::Drawer::Instance();
                drawer.Toggle(!drawer.IsActive());
                std::cout << "[DebugDraw] Debug mode: " << (drawer.IsActive() ? "ON" : "OFF") << "\n";

                debugToggled = true; // prevent double toggle on hold
            }
        }
        
        else if (keyMsg->action == GLFW_RELEASE) {
           // std::cout << "[GameLoop] Key released: " << keyMsg->keyCode << "\n";

            // Only reset debounce when releasing the same key
            if (keyMsg->keyCode == GLFW_KEY_B) {
                debugToggled = false;
            }
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
