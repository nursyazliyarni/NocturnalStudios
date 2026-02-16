/*!------------------------------------------------------------------------------------------------
	File: EndScreenDisplay.cpp
	Project: NocturnalStudio

	Brief:
		The EndScreenDisplay system manages the displaying of the Win/lose game. It adds stats,
		and then makes the stuff slide in fancily. Its attached to the bachground object

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
			p_ecs.GetComponent<Transform>(e).MoveX(2.3f);
		}

		fileT=p_ecs.GetComponent<Transform>(fileBG);
		p_ecs.GetComponent<Transform>(fileBG).MoveX(-2.f);


		if (GetEntityByName("Payout")!=0&&p_ecs.HaveComponent<Text>( GetEntityByName("Payout")))
		{
			int basePay = 300;
			basePay -= p_GameManager.AnimalsGone() * 100;
			basePay -= p_GameManager.getCrits() * 10;
			if (basePay < 0)basePay = 0;

			p_ecs.GetComponent<Text>(GetEntityByName("Payout")).SetContent("Your payout: @$"+std::to_string(basePay)+"|");
		}
		if (p_ecs.HaveComponent<Text>(escapedStats))p_ecs.GetComponent<Text>(escapedStats).SetContent("Number of animals alive/in their\nenclosures: "+std::to_string(4-p_GameManager.AnimalsGone())+"/4");
		if (p_ecs.HaveComponent<Text>(barStats))p_ecs.GetComponent<Text>(barStats).SetContent("Number of times the bars have reached\nbelow 20%: " + std::to_string(p_GameManager.getCrits()));
		
	}

	void EndScreenDisplay::Update(float Frametime)
	{
		

		if (allOtherT<timeToFloatIn)
		{
			for (std::pair<Entity, Transform> ep:paper)
			{
				float xdif{ ep.second.GetPos().x - p_ecs.GetComponent<Transform>(ep.first).GetPos().x };
				p_ecs.GetComponent<Transform>(ep.first).MoveX(xdif / (timeToFloatIn - allOtherT) * Frametime);
			}
			allOtherT += Frametime;
		}
		if (bgT < timeToFloatIn)
		{
			float xdif{ fileT.GetPos().x - p_ecs.GetComponent<Transform>(fileBG).GetPos().x };
			p_ecs.GetComponent<Transform>(fileBG).MoveX(xdif / (timeToFloatIn - bgT) * Frametime);
			bgT += Frametime;
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
