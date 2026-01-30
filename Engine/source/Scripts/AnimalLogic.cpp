/*!------------------------------------------------------------------------------------------------
	File: AnimalLogic.cpp
	Project: NocturnalStudio

	Brief:
		The AnimalLogic system manages the displaying of Various AnimalLogics of Anumals Bars of an

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the AnimalLogic class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/AnimalLogic.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"


namespace NocturnalEngine {

	void AnimalLogic::Start()
	{

		//This is so that when the game entity withthis script is initiated,
		//the values are loaded in from the gamemanager. 
		if (p_ecs.HaveComponent<NameTag>(entityID)&& p_ecs.HaveComponent<AnimalAttributes>(entityID)) {
			NameTag& nameT{ p_ecs.GetComponent<NameTag>(entityID) };
			std::string name = nameT.GetNameTag();
			GameManager::GameScene gS{ GameManager::GameScene::NONE };
			if (name=="Flamingo")
			{
				gS = GameManager::GameScene::FLAMINGO;
			}
			if (name == "Horse")
			{
				gS = GameManager::GameScene::HORSE;
			}
			if (name == "Seal")
			{
				gS = GameManager::GameScene::SEAL;
			}
			if (name == "Owl")
			{
				gS = GameManager::GameScene::OWL;
			}
			AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };
			// in addition, the gamemanager also saves the above gS as the current gamestate
			aA=p_GameManager.LoadPersistentValues(gS);
		}
	}

	void AnimalLogic::Update(float Frametime)
	{
		//by right, the entity this script is connected to NEEDS to have
		//the AnimalAttributes Component to have this script. This is just 
		//a safeguard in case
		if (!p_ecs.HaveComponent<AnimalAttributes>(entityID))return;

		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };
		
		//same as original code
		if (aA.is_alive) {
			aA.current_hunger -= aA.hungerDecayRate * static_cast<double>(Frametime) * 10.0;
			aA.current_health -= aA.healthDecayRate * static_cast<double>(Frametime);
			aA.current_sanity -= aA.sanityDecayRate * static_cast<double>(Frametime);


			if (aA.current_hunger <= 0 || aA.current_health <= 0 || aA.current_sanity <= 0) {
				aA.is_alive = false;

			}
		}


	}

	void AnimalLogic::End()
	{
		// this is so that the state saves to the game manager upon unloading
		if (!p_ecs.HaveComponent<AnimalAttributes>(entityID))return;

		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };
		p_GameManager.SaveAnimalAttributesToCurrentScene(aA);

	}




}
