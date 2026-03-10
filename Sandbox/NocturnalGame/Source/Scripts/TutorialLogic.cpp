#include "pch.h"
#include "Scripts/Tutorial_Logic.h"
#include "ECS/ECS.h"

namespace NocturnalEngine {

	void TutorialLogic::Start()
	{

	}

	void TutorialLogic::Update(float Frametime)
	{
		if (p_ecs.HaveComponent<Text>(entityID)) {
			auto& text{ p_ecs.GetComponent<Text>(entityID) };
			/*if (InputSystem::GetInstance()->MousePressed(0)&&!text.GET_mContent_array().empty()) {
				text.GET_mContent_array().erase(text.GET_mContent_array().begin());
			}*/
		}
	}

	void TutorialLogic::End()
	{
		
	}

}