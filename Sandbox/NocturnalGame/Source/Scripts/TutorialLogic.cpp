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
#include "ECS/Components/ButtonComponent.h"
#include "ECS/ECS.h"
#include <GLFW/glfw3.h>

namespace NocturnalEngine {

	namespace
	{
		constexpr int kFinalTutorialOrder = 11;
		int sCurrentTutorialOrder = 0;

		std::string ResolveTutorialAnimalTexture(Sprite& sprite, int tutorialOrder)
		{
			if (sprite.stateRules.empty())
			{
				return sprite.GetTexture();
			}

			size_t stateIndex = 0;
			switch (tutorialOrder)
			{
			case 3:
			case 4:
				stateIndex = 1;
				break;
			case 5:
			case 6:
				stateIndex = 2;
				break;
			case 7:
			case 8:
				stateIndex = 3;
				break;
			default:
				stateIndex = 0;
				break;
			}

			if (stateIndex >= sprite.stateRules.size() ||
				sprite.stateRules[stateIndex].textureKey.empty())
			{
				return sprite.stateRules.front().textureKey.empty()
					? sprite.GetTexture()
					: sprite.stateRules.front().textureKey;
			}

			return sprite.stateRules[stateIndex].textureKey;
		}
	}

	void TutorialLogic::Start()
	{
		sCurrentTutorialOrder = 0;

		if (p_ecs.HaveComponent<TutorialAttributes>(entityID) && p_ecs.HaveComponent<Text>(entityID)) {
			if (p_ecs.HaveComponent<ButtonComponent>(entityID)) {
				auto& button{ p_ecs.GetComponent<ButtonComponent>(entityID) };
				button.SET_button_logic_ON(false);
			}

			auto& tutorial{ p_ecs.GetComponent<TutorialAttributes>(entityID) };
			auto& text{ p_ecs.GetComponent<Text>(entityID) };
			if (tutorial.tutorial_order == sCurrentTutorialOrder) {
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
			if (tutorial.tutorial_order == 100)
			{
				InputSystem* input = InputSystem::GetInstance().get();
				if (!input)
				{
					return;
				}

				if (input->KeyPressed(GLFW_KEY_X)) {
					sCurrentTutorialOrder = kFinalTutorialOrder;
				}
				else if (sCurrentTutorialOrder < kFinalTutorialOrder && input->MousePressed(0)) {
					++sCurrentTutorialOrder; //left click to move to next tutorial
				}
			}

			if (sCurrentTutorialOrder >= kFinalTutorialOrder) { //this means we have reached the last tutorial
				sCurrentTutorialOrder = kFinalTutorialOrder;
				if (p_ecs.HaveComponent<ButtonComponent>(entityID)) {
					auto& button{ p_ecs.GetComponent<ButtonComponent>(entityID) };
					button.SET_button_logic_ON(true); //now button response logic is running for last tutorial
				}
			}

			if (p_ecs.HaveComponent<Text>(entityID)) {
				auto& text{ p_ecs.GetComponent<Text>(entityID) };
				if (tutorial.tutorial_order == sCurrentTutorialOrder) {
					text.SET_visibility(true);
				}

				else {
					text.SET_visibility(false);
				}
			}
				
			if (p_ecs.HaveComponent<Sprite>(entityID) && tutorial.tutorial_order == 100) {
				auto& sprite{ p_ecs.GetComponent<Sprite>(entityID) };
				sprite.GetTexture() = ResolveTutorialAnimalTexture(sprite, sCurrentTutorialOrder);
			}
		}
	}

	void TutorialLogic::End()
	{
		sCurrentTutorialOrder = 0;
	}

}
