/*!------------------------------------------------------------------------------------------------
	File: SelfDestruct.cpp
	Project: NocturnalStudio

	Brief:
		The SelfDestruct converts elapsed gametime to ingame time and then displays it into a text
		component
	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the SelfDestruct class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/SelfDestruct.h"
#include "ECS/ECS.h"
#include "ECS/ParentingSystem.h"


namespace NocturnalEngine {

	void SelfDestruct::Start()
	{
		

	}

	void SelfDestruct::Update(float Frametime)
	{
		timeSince += Frametime;
		float timeleft{ timeTillDie - timeSince };
		if (timeSince>=timeTillDie)
		{
			if (p_ecs.HaveComponent<Sprite>(entityID))
			{
				p_ecs.GetComponent<Sprite>(entityID).SetAlpha(0.f);
			}
			if (p_ecs.HaveComponent<Text>(entityID))p_ecs.GetComponent<Text>(entityID).SetContent("");
		}
		else {
			if (p_ecs.HaveComponent<Sprite>(entityID))
			{
				p_ecs.GetComponent<Sprite>(entityID).SetAlpha(1.f*(timeleft/timeTillDie));
			}
			if (p_ecs.HaveComponent<Text>(entityID)) { 
				float* c = p_ecs.GetComponent<Text>(entityID).GetColor();
				p_ecs.GetComponent<Text>(entityID).SetColor(c[0],c[1],c[2],1.f * (timeleft / timeTillDie));
			}
		}

	}

	void SelfDestruct::End()
	{



	}




}
