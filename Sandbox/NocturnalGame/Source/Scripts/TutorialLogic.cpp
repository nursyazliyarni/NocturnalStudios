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
		static unsigned int current_index{ 0 };
		if (p_ecs.HaveComponent<Text>(entityID)) {
			auto& text{ p_ecs.GetComponent<Text>(entityID) };
			if (InputSystem::GetInstance()->MousePressed(0)&&!text.GET_mContent_array().empty()) {
				text.GET_mContent_array().erase(text.GET_mContent_array().begin());
				text.GET_position_vector().erase(text.GET_position_vector().begin());
				current_index++;
			}
		}

		if (p_ecs.HaveComponent<Sprite>(entityID)) {
			
			auto& sprite{ p_ecs.GetComponent<Sprite>(entityID) };
			//idle,hungry,sick,angry(4 states for animal)
			sprite.GetTexture() = sprite.stateRules[3].textureKey;
		}
	}

	void TutorialLogic::End()
	{
		
	}

}