/*!------------------------------------------------------------------------------------------------
	File: AnimalLogic.cpp
	Project: NocturnalStudio

	Brief:
		The AnimalLogic system manages the displaying of Various AnimalLogics of Anumals Bars of an

	Primary Author:
		 ANG YU XUAN (70%)
		- Created the AnimalLogic class and its function specification.
    Secondary Author:
         Nur Syazliyarni (5%)
        - Adjusted health decay behavior per animal.
		Sarah (25%)
		- Make improvements and fix error with this AnimalLogic script.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/AnimalLogic.h"
#include "ECS/ECS.h"
#include "Scripts/GlobalGameManager.h"
#include "ECS/Components/AnimalAttributes.h"


namespace NocturnalEngine {

	void AnimalLogic::Start()
	{

		//This is so that when the game entity withthis script is initiated,
		//the values are loaded in from the gamemanager. 
		if (p_ecs.HaveComponent<NameTag>(entityID)&& p_ecs.HaveComponent<AnimalAttributes>(entityID)) {
			NameTag& nameT{ p_ecs.GetComponent<NameTag>(entityID) };
			std::string name = nameT.GetNameTag();

			AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };

			orgXPos = p_ecs.GetComponent<Transform>(entityID).GetPos().x;

			// in addition, the gamemanager also saves the above gS as the current gamestate
			aA=p_GameManager.LoadPersistentValues(name);
		}
	}

	void AnimalLogic::Update(float Frametime)
	{
		MoveAnimalLeftRight(Frametime);
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

		if (!aA.isEscaped&&aA.is_alive&&p_GameManager.GetGameState() != GameManager::GameState::ENDING) {
			aA.current_hunger -= aA.hungerDecayRate * static_cast<double>(Frametime) * 2.0;

			// Adjust health decay based on animal type and hunger state
			const double baseHealthMultiplier = 3.0; // much faster overall
			double healthMultiplier = baseHealthMultiplier;
			if (p_ecs.HaveComponent<NameTag>(entityID))
			{
				const std::string& name = p_GameManager.IsNewSceneManagement()?aA.WhatKindOfAnimalIsThis():p_ecs.GetComponent<NameTag>(entityID).GetNameTag();
				const double hungerRatio = (aA.maxHunger > 0.0) ? (aA.current_hunger / aA.maxHunger) : 0.0;
				const double lowHungerThreshold = 0.30;   // below 30% is "low"
				const double highHungerThreshold = 0.70;  // above 70% is "normal+"

				if (name == "Flamingo")
				{
					if (hungerRatio < lowHungerThreshold)
						healthMultiplier = 2.0; // faster when hungry
					else if (hungerRatio > highHungerThreshold)
						healthMultiplier = 0.5; // slower when well fed
				}
				else if (name == "Horse")
				{
					healthMultiplier = 1.5; // faster health decay
				}
				// Seal and Owl remain at normal decay (1.0)
			}

			aA.current_health -= aA.healthDecayRate * static_cast<double>(Frametime) * healthMultiplier;
			aA.current_sanity -= aA.sanityDecayRate * static_cast<double>(Frametime);

			if (aA.current_sanity <= 0)
			{
				aA.isEscaped = true;

				p_GameManager.AddEscape();
			}
			if (aA.current_hunger <= 0 || aA.current_health <= 0 ) {
				aA.is_alive = false;
				p_GameManager.AddDeath();
			}
			LogLowHealth(aA);
		}
		else if (aA.isEscaped&&!spriteDeactivated) {
			p_ecs.GetComponent<Sprite>(entityID).SetTexture("Characters/Blank");
			spriteDeactivated = true;
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
		std::string animalName = p_ecs.GetComponent<NameTag>(entityID).GetNameTag() == "Animal" ? aA.WhatKindOfAnimalIsThis() : p_ecs.GetComponent<NameTag>(entityID).GetNameTag();

		if (aA.current_health <= aA.maxHealth / 2.0 && aA.healthWarnLevel<1)
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Health is below 50%!");
			aA.healthWarnLevel++;
		}
		else if (aA.current_health <= aA.maxHealth / 5.0 && aA.healthWarnLevel < 2) 
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Health is below 20%!");
			aA.healthWarnLevel++;
			p_GameManager.AddCrit();
		}
		else if (aA.current_health > aA.maxHealth / 2.0 && aA.healthWarnLevel!=0) {
			aA.healthWarnLevel = 0;
		}
		if (aA.current_hunger <= aA.maxHunger / 2.0 && aA.hungerWarnLevel<1)
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Hunger is below 50%!");
			aA.hungerWarnLevel++;
		}
		else if (aA.current_hunger <= aA.maxHunger / 5.0 && aA.hungerWarnLevel < 2)
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Hunger is below 20%!");
			aA.hungerWarnLevel++;
			p_GameManager.AddCrit();
		}
		else if (aA.current_hunger > aA.maxHunger / 2.0 && aA.hungerWarnLevel!=0) {
			aA.hungerWarnLevel = 0;
		}
		if (aA.current_sanity <= aA.maxSanity / 2.0 && aA.sanityWarnLevel<1)
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Sanity is below 50%!");
			aA.sanityWarnLevel++;
		}
		else if (aA.current_sanity <= aA.maxSanity / 5.0 && aA.sanityWarnLevel < 2)
		{
			p_GameManager.AddLogEntry("WARNING! " + animalName + "'s Sanity is below 20%!");
			aA.sanityWarnLevel++;
			p_GameManager.AddCrit();
		}
		else if (aA.current_sanity > aA.maxSanity / 2.0 && aA.sanityWarnLevel!=0) {
			aA.sanityWarnLevel = 0;
		}

		if (!aA.is_alive)
		{
			p_GameManager.AddLogEntry( animalName + " is dead.");
		}
		if (aA.isEscaped)
		{
			p_GameManager.AddLogEntry(animalName + " has breached containment.");
		}
	}

	void AnimalLogic::MoveAnimalLeftRight(float Frametime) {
		UNREFERENCED_PARAMETER(Frametime);

		// Transform& t{ p_ecs.GetComponent<Transform>(entityID) };

	}

}
