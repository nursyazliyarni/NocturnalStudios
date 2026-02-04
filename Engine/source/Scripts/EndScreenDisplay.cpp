/*!------------------------------------------------------------------------------------------------
	File: EndScreenDisplay.cpp
	Project: NocturnalStudio

	Brief:
		The EndScreenDisplay system manages the displaying of Various EndScreenDisplays of Anumals Bars of an

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the EndScreenDisplay class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/EndScreenDisplay.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	void EndScreenDisplay::Start()
	{
		fileBG = GetEntityByName("FileBG");
		dayE = GetEntityByName("DayIndicator");
		escapedStats= GetEntityByName("EscapedStats");
		barStats = GetEntityByName("BarStats");
		const std::set<Entity> sete{ p_ecs.GetAliveEntitiesSet() };
		for (Entity e:sete)
		{
			if (e == entityID || e == fileBG)continue;
			if (p_ecs.HaveComponent<Transform>(e))paper.emplace_back(e, p_ecs.GetComponent<Transform>(e));
			
		}

		if (p_ecs.HaveComponent<Text>(escapedStats))p_ecs.GetComponent<Text>(escapedStats).SetContent("Number of animals alive/in their\nenclosures: "+std::to_string(4-p_GameManager.AnimalsGone())+"/4");
		if (p_ecs.HaveComponent<Text>(barStats))p_ecs.GetComponent<Text>(barStats).SetContent("Number of times the bars have reached\nbelow 20%: " + std::to_string(p_GameManager.getCrits()));
		
	}

	void EndScreenDisplay::Update(float Frametime)
	{
		if (bgT<timeToFloatIn)
		{
			p_ecs.fer
		}




	}

	void EndScreenDisplay::End()
	{



	}


	Entity EndScreenDisplay::GetEntityByName(const std::string name) {
		std::set<Entity> eSet{ p_ecs.GetAliveEntitiesSet() };

		for (Entity e : eSet)
		{
			if (p_ecs.HaveComponent<NameTag>(e) && p_ecs.GetComponent<NameTag>(e).GetNameTag() == name) return e;
		}
		return 0;
	}

}
