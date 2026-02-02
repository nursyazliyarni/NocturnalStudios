/*!------------------------------------------------------------------------------------------------
	File: AnimalLogic.cpp
	Project: NocturnalStudio

	Brief:
		The AnimalLogic system manages the displaying of Various AnimalLogics of Anumals Bars of an

	Primary Author:
		 ANG YU XUAN (100%)
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
		//std::cout << "Entity " << p_ecs.GetComponent<NameTag>(entityID).GetNameTag() << " script running\n";
		
		//by right, the entity this script is connected to NEEDS to have
		//the AnimalAttributes Component to have this script. This is just 
		//a safeguard in case
		if (!p_ecs.HaveComponent<AnimalAttributes>(entityID))return;

		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };
		if (p_GameManager.GetGameState() == GameManager::GameState::STARTING)
		{
			aA.SetDefault();
		}
		//same as original code
		if (aA.is_alive&&p_GameManager.GetGameState() != GameManager::GameState::ENDING) {
			aA.current_hunger -= aA.hungerDecayRate * static_cast<double>(Frametime) * 10.0;
			aA.current_health -= aA.healthDecayRate * static_cast<double>(Frametime);
			aA.current_sanity -= aA.sanityDecayRate * static_cast<double>(Frametime);

			if (aA.current_hunger <= 0 || aA.current_health <= 0 || aA.current_sanity <= 0) {
				aA.is_alive = false;
				p_GameManager.AddDeath();
			}
			LogLowHealth(aA);
		}
		else if (p_GameManager.GetGameState() == GameManager::GameState::ENDING) {
			aA.SetDefault();
			p_GameManager.SaveAnimalAttributesToCurrentScene(aA);
		}


	}

	void AnimalLogic::End()
	{
		// this is so that the state saves to the game manager upon unloading
		if (!p_ecs.HaveComponent<AnimalAttributes>(entityID))return;

		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };
		if (p_GameManager.GetGameState() == GameManager::GameState::ENDING)
		{
			aA.SetDefault();
		}
		p_GameManager.SaveAnimalAttributesToCurrentScene(aA);
		

	}


	void AnimalLogic::LogLowHealth(AnimalAttributes& aA) {
		if (aA.current_health <= aA.maxHealth / 2.0 && !aA.healthWarn)
		{
			p_GameManager.AddLogEntry("WARNING! " + p_ecs.GetComponent<NameTag>(entityID).GetNameTag() + "'s Health is below 50%!");
			aA.healthWarn = true;
		}
		else if (aA.current_health > aA.maxHealth / 2.0 && aA.healthWarn) {
			aA.healthWarn = false;
		}
		if (aA.current_hunger <= aA.maxHunger / 2.0 && !aA.hungerWarn)
		{
			p_GameManager.AddLogEntry("WARNING! " + p_ecs.GetComponent<NameTag>(entityID).GetNameTag() + "'s Hunger is below 50%!");
			aA.hungerWarn = true;
		}
		else if (aA.current_hunger > aA.maxHunger / 2.0 && aA.hungerWarn) {
			aA.hungerWarn = false;
		}
		if (aA.current_sanity <= aA.maxSanity / 2.0 && !aA.sanityWarn)
		{
			p_GameManager.AddLogEntry("WARNING! " + p_ecs.GetComponent<NameTag>(entityID).GetNameTag() + "'s Sanity is below 50%!");
			aA.sanityWarn = true;
		}
		else if (aA.current_sanity > aA.maxSanity / 2.0 && aA.sanityWarn) {
			aA.sanityWarn = false;
		}

		if (!aA.is_alive)
		{
			p_GameManager.AddLogEntry( p_ecs.GetComponent<NameTag>(entityID).GetNameTag() + "is dead.");
		}
	}

}
