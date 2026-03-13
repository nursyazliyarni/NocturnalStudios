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
#include "ECS/ECS.h"

namespace NocturnalEngine {

	void TutorialLogic::Start()
	{
		if (p_ecs.HaveComponent<Text>(entityID)) {
			p_ecs.GetComponent<Text>(entityID).Set_current_index(0);
		}
	}

	void TutorialLogic::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);
		static int tutorial_current_index{ 0 };

		if (p_ecs.HaveComponent<Text>(entityID)) {
			auto& text{ p_ecs.GetComponent<Text>(entityID) };

			if (InputSystem::GetInstance()->MousePressed(0)&&!text.GET_mContent_array().empty()) {
				text.GET_mContent_array().erase(text.GET_mContent_array().begin());
				text.GET_position_vector().erase(text.GET_position_vector().begin());
				tutorial_current_index++;
			}
		}

		if (p_ecs.HaveComponent<Sprite>(entityID)) {
			auto& sprite{ p_ecs.GetComponent<Sprite>(entityID) };
			//idle,hungry,sick,angry(4 states for animal)
			switch (tutorial_current_index) {
			case 3: //hungry
				sprite.GetTexture() = sprite.stateRules[1].textureKey; break;
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

	void TutorialLogic::End()
	{
		
	}

}