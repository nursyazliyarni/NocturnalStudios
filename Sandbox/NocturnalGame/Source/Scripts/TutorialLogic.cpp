/*!------------------------------------------------------------------------------------------------
	File: TutorialLogic.cpp
	Project: NocturnalStudio

	Brief:
		This file handles the gameplay tutorial for tutorial.json

	Primary Author:
		 ANG YU XUAN(100%)
		- Created functions to handles the gameplay tutorial logic


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/TutorialLogic.h"
#include "ECS/Components/TutorialAttributes.h"
#include "ECS/ECS.h"

namespace NocturnalEngine {

	void TutorialLogic::Start()
	{
		if (p_ecs.HaveComponent<TutorialAttributes>(entityID) && p_ecs.HaveComponent<Text>(entityID)) {
			auto& tutorial{ p_ecs.GetComponent<TutorialAttributes>(entityID) };
			auto& text{ p_ecs.GetComponent<Text>(entityID) };
			if (tutorial.tutorial_order == current_tutorial_order) {
				text.SET_visibility(true);
			}

			else { //this is to ensure that only the first gameplay tutorial is displayed on the screen
				text.SET_visibility(false);
			}
		}
	}

	void TutorialLogic::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);
		if (p_ecs.HaveComponent<TutorialAttributes>(entityID)) {
			auto& tutorial{ p_ecs.GetComponent<TutorialAttributes>(entityID) };
			if (InputSystem::GetInstance()->MousePressed(0)) {
				current_tutorial_order++; //left click to move to next tutorial
			}

			if (p_ecs.HaveComponent<Text>(entityID)) {
				auto& text{ p_ecs.GetComponent<Text>(entityID) };
				if (tutorial.tutorial_order == current_tutorial_order) {
					text.SET_visibility(true);
				}

				else {
					text.SET_visibility(false);
				}
			}
				
			if (p_ecs.HaveComponent<Sprite>(entityID) && tutorial.tutorial_order == 100) {
				auto& sprite{ p_ecs.GetComponent<Sprite>(entityID) };
				//idle,hungry,sick,angry(4 states for animal)
				switch (current_tutorial_order) {
				case 3: //hungry
					sprite.GetTexture() = sprite.stateRules[1].textureKey;
					break;
				case 4: //hungry
					sprite.GetTexture() = sprite.stateRules[1].textureKey; break;
				case 5: //sick
					sprite.GetTexture() = sprite.stateRules[2].textureKey; break;
				case 6: //sick
					sprite.GetTexture() = sprite.stateRules[2].textureKey; break;
				case 7: //angry
					sprite.GetTexture() = sprite.stateRules[3].textureKey; break;
				case 8: //angry
					sprite.GetTexture() = sprite.stateRules[3].textureKey; break;
				default: sprite.GetTexture() = sprite.stateRules[0].textureKey; break; //idle
				}
			}
		}
	}

	void TutorialLogic::End()
	{
		
	}

}