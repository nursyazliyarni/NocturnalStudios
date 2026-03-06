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
#include "Math/Random.h"
#include "ResourceManager/ResourceManager.h"
#include "LevelEditor/LevelEditor.h"


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
			GameManager::AnimalData aD= p_GameManager.LoadPersistentValues(name);
			// in addition, the gamemanager also saves the above gS as the current gamestate
			aA = aD.aA;
			chance = { aD.hungerD, aD.hpD, aD.sanityD };
			time = { aD.hungerT, aD.hpT, aD.sanityT };

			//fetch checkperiod
			checkPeriod = p_GameManager.GetCheckPeriod();

			if (p_ecs.HaveComponent<Sprite>(entityID))
			{
				auto& sprite = p_ecs.GetComponent<Sprite>(entityID);
				ogSpritename = sprite.GetTexture();
				if (sprite.stateManagerEnabled && !sprite.stateIdleTexture.empty())
					ogSpritename = sprite.stateIdleTexture;
			}
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

		//3.0 base health decay rate

		//Flamingo's health decreases faster based on its hunger. If hunger is above 70%, health decreases at a rate of 0.5.
		//If hunger is below 30%, health decreases at 2x the rate

		//horse is just 1.5x faster



		AnimalAttributes& aA{ p_ecs.GetComponent<AnimalAttributes>(entityID) };

		


		if (!aA.isEscaped&&aA.is_alive&&p_GameManager.GetGameState() != GameManager::GameState::ENDING) {

			//hunger>hp>sanity
			CheckAndDecreaseStats(time.hunger, chance.hunger, aA.hungerDecayRate, aA.current_hunger, Frametime);
			
			double hpDecay = aA.healthDecayRate,decreaseScale = 1.0;
			if (aA.WhatKindOfAnimalIsThis() == "Flamingo")
			{
				if (aA.current_hunger > aA.maxHunger * 0.7) { hpDecay *= 0.5; decreaseScale = 0.7; }
				else if (aA.current_hunger <= aA.maxHunger * 0.3) { hpDecay *= 2.0; decreaseScale = 2.0; }
			}
			if (aA.WhatKindOfAnimalIsThis() == "Horse")
			{
				if (aA.current_hunger<= aA.maxHunger * 0.8) { hpDecay *= 1.5; decreaseScale = 1.5; }
			}

			CheckAndDecreaseStats(time.hp, chance.hp, hpDecay, aA.current_health, Frametime,decreaseScale);

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
			ChangeAnim(aA);
		}
		else if (aA.isEscaped&&!spriteDeactivated) {
			p_ecs.GetComponent<Sprite>(entityID).SetTexture("Characters/Blank");
			spriteDeactivated = true;
			std::cout << "Escaped\n";
		}
		else if (p_GameManager.GetGameState() == GameManager::GameState::ENDING) {
			aA.SetDefault();
			p_GameManager.SaveAnimalAttributesToCurrentScene({ aA,chance.hunger,chance.hp,chance.sanity,time.hunger,time.hp,time.sanity });
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
		p_GameManager.SaveAnimalAttributesToCurrentScene({ aA,chance.hunger,chance.hp,chance.sanity,time.hunger,time.hp,time.sanity });
		

	}

	void AnimalLogic::ChangeAnim(AnimalAttributes& aA) {
		if (ogSpritename.empty() || !p_ecs.HaveComponent<Sprite>(entityID))
			return;
		if (aA.isEscaped || !aA.is_alive)
			return;

		auto& sprite = p_ecs.GetComponent<Sprite>(entityID);
		if (!sprite.stateManagerEnabled)
			return;
		std::string stateName = "idle";
		const std::string idleTex = sprite.stateIdleTexture.empty() ? ogSpritename : sprite.stateIdleTexture;
		const std::string hungryTex = sprite.stateHungryTexture.empty() ? (ogSpritename + "_hungry") : sprite.stateHungryTexture;
		const std::string sickTex = sprite.stateSickTexture.empty() ? (ogSpritename + "_sick") : sprite.stateSickTexture;
		const std::string angryTex = sprite.stateAngryTexture.empty() ? (ogSpritename + "_angry") : sprite.stateAngryTexture;
		std::string desiredTexture = idleTex;

		const double hungerRatio = aA.maxHunger > 0.0 ? (aA.current_hunger / aA.maxHunger) : 1.0;
		const double healthRatio = aA.maxHealth > 0.0 ? (aA.current_health / aA.maxHealth) : 1.0;
		const double sanityRatio = aA.maxSanity > 0.0 ? (aA.current_sanity / aA.maxSanity) : 1.0;
		const double hungryThreshold = glm::clamp(static_cast<double>(sprite.stateHungryThreshold), 0.0, 1.0);
		const double sickThreshold = glm::clamp(static_cast<double>(sprite.stateSickThreshold), 0.0, 1.0);
		const double angryThreshold = glm::clamp(static_cast<double>(sprite.stateAngryThreshold), 0.0, 1.0);

		const bool editorPreview = (p_Editor != nullptr) && p_Editor->is_ShowWindow && sprite.statePreviewInEditor && sprite.statePreviewMode > 0;
		if (editorPreview)
		{
			switch (sprite.statePreviewMode)
			{
			case 1: stateName = "idle"; break;
			case 2: stateName = "hungry"; break;
			case 3: stateName = "sick"; break;
			case 4: stateName = "angry"; break;
			default: stateName = "idle"; break;
			}
		}
		else
		{
			const double angryScore = sanityRatio <= angryThreshold ? sanityRatio : 2.0;
			const double sickScore = healthRatio <= sickThreshold ? healthRatio : 2.0;
			const double hungryScore = hungerRatio <= hungryThreshold ? hungerRatio : 2.0;

			if (angryScore <= sickScore && angryScore <= hungryScore && angryScore <= 1.0)
			{
				stateName = "angry";
			}
			else if (sickScore <= hungryScore && sickScore <= 1.0)
			{
				stateName = "sick";
			}
			else if (hungryScore <= 1.0)
			{
				stateName = "hungry";
			}
		}

		if (stateName == "hungry") desiredTexture = hungryTex;
		else if (stateName == "sick") desiredTexture = sickTex;
		else if (stateName == "angry") desiredTexture = angryTex;

		// Guard against invalid texture keys causing null texture dereference in Animation.
		if (!ResourceManager::GetTexture(desiredTexture))
		{
			if (stateName == "hungry")
			{
				if (ResourceManager::GetTexture("Characters/flamingo_hungry")) desiredTexture = "Characters/flamingo_hungry";
				else if (ResourceManager::GetTexture("Characters/flamingo_spritesheet_hungry")) desiredTexture = "Characters/flamingo_spritesheet_hungry";
				else desiredTexture = idleTex;
			}
			else if (stateName == "sick")
			{
				if (ResourceManager::GetTexture("Characters/flamingo_sick")) desiredTexture = "Characters/flamingo_sick";
				else if (ResourceManager::GetTexture("Characters/flamingo_spritesheet_sick")) desiredTexture = "Characters/flamingo_spritesheet_sick";
				else desiredTexture = idleTex;
			}
			else if (stateName == "angry")
			{
				if (ResourceManager::GetTexture("Characters/flamingo_angry")) desiredTexture = "Characters/flamingo_angry";
				else if (ResourceManager::GetTexture("Characters/flamingo_spritesheet_crazy")) desiredTexture = "Characters/flamingo_spritesheet_crazy";
				else desiredTexture = idleTex;
			}
			else
			{
				if (ResourceManager::GetTexture("Characters/flamingo_idle")) desiredTexture = "Characters/flamingo_idle";
				else desiredTexture = idleTex;
			}
		}

		if (sprite.GetTexture() != desiredTexture)
		{
			sprite.SetTexture(desiredTexture);
			int x = 0;
			int y = sprite.GetIndex().y;
			sprite.SetIndex(x, y);
			sprite.internaltimer = 0.f;
		}

	}

	void AnimalLogic::LogLowHealth(AnimalAttributes& aA) {
		std::string animalName = p_ecs.GetComponent<NameTag>(entityID).GetNameTag() == "Animal" ? aA.WhatKindOfAnimalIsThis() : p_ecs.GetComponent<NameTag>(entityID).GetNameTag();

		if (aA.current_health <= aA.maxHealth / 2.0 && aA.healthWarnLevel<1)
		{

			aA.healthWarnLevel++;
		}
		else if (aA.current_health <= aA.maxHealth / 5.0 && aA.healthWarnLevel < 2) 
		{
			p_GameManager.AddCrit();
			aA.healthWarnLevel++;
		}
		else if (aA.current_health > aA.maxHealth / 2.0 && aA.healthWarnLevel!=0) {

			aA.healthWarnLevel = 0;
		}
		if (aA.current_hunger <= aA.maxHunger / 2.0 && aA.hungerWarnLevel<1)
		{

			aA.hungerWarnLevel++;
		}
		else if (aA.current_hunger <= aA.maxHunger / 5.0 && aA.hungerWarnLevel < 2)
		{
			aA.hungerWarnLevel++;
			p_GameManager.AddCrit();
		}
		else if (aA.current_hunger > aA.maxHunger / 2.0 && aA.hungerWarnLevel!=0) {
			aA.hungerWarnLevel = 0;
		}
		if (aA.current_sanity <= aA.maxSanity / 2.0 && aA.sanityWarnLevel<1)
		{
			aA.sanityWarnLevel++;
		}
		else if (aA.current_sanity <= aA.maxSanity / 5.0 && aA.sanityWarnLevel < 2)
		{
			aA.sanityWarnLevel++;
			p_GameManager.AddCrit();
		}
		else if (aA.current_sanity > aA.maxSanity / 2.0 && aA.sanityWarnLevel!=0) {
			aA.sanityWarnLevel = 0;

		}

		if (!aA.is_alive)
		{
			p_GameManager.AddLogEntry( animalName + " is not moving");
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
	void AnimalLogic::CheckAndDecreaseStats(float& times, float& chances, double decayRate, double& attributeValue,float frametime, double decreaseScale) {
		if (decayRate <= 0.0)decayRate = 0.00000001;
		if (times >= checkPeriod / decayRate)
		{
			//std::cout << "Checking\n";
			float bruh{ Random::Range(0.f, 100.f) };
			if ( bruh<= chances)
			{
				//std::cout << bruh << "\n";
				attributeValue -= Random::Range(5.f, 15.f)*decreaseScale;
				chances = 10.f;
			}
			else {
				chances += 10.f;
			}
			times = 0.f;
		}
		else
		{
			times += frametime;
		}
	}
}
