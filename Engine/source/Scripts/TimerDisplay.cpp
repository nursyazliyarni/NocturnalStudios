/*!------------------------------------------------------------------------------------------------
	File: TimerDisplay.cpp
	Project: NocturnalStudio

	Brief:
		The TimerDisplay system manages the displaying of Various TimerDisplays of Anumals Bars of an

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the TimerDisplay class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/TimerDisplay.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	void TimerDisplay::Start()
	{
		timeScale = p_GameManager.GetTimeScale();
	
	}

	void TimerDisplay::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);


		if (!p_ecs.HaveComponent<Text>(entityID)) return;

		Text& txt{ p_ecs.GetComponent<Text>(entityID) };
		//06:45:23

		if (timeScale==0)timeScale = p_GameManager.GetTimeScale();

		float timeInGameWorld{ p_GameManager.GetTimeElapsed() * timeScale };
		int hours{ static_cast<int>(timeInGameWorld / 3600.f) },
			mins{ static_cast<int>((timeInGameWorld - (static_cast<float>(hours) * 3600.f)) / 60.f) },
			secs{ static_cast<int>(timeInGameWorld - (static_cast<float>(hours) * 3600.f) - (static_cast<float>(mins) * 60.f)) };

		char buffer[16];
		sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
		txt.SetContent(std::string{buffer});

	}

	void TimerDisplay::End()
	{



	}




}
