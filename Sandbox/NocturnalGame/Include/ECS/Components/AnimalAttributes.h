/*!------------------------------------------------------------------------------------------------
    File: AnimalAttributes.h
    Project: NocturnalStudio

	Brief:
		Contains the attributes related to animals in the ECS
		
    Primary Author:
         Ang Yu Xuan (100%)
		- Created the AnimalAttributes.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Components/IComponent.h"
#include "ECS/SceneManager.h"
#include "pch.h"

namespace NocturnalEngine
{
	class AnimalAttributes : public IComponent
	{
	public:
		enum class AnimalType {
			FLAMINGO,
			SEAL, 
			HOESE,
			OWL
		};
		enum class PriorityAnimation {
			NORMAL,
			HUNGRY,
			SICK,
			CRAZY
		};
	private:
		AnimalType animalType{ AnimalType::FLAMINGO };
		PriorityAnimation anim{ PriorityAnimation::NORMAL };
	public:
		void SetAnimalType(int a) {
			switch (a) {
			case 1: animalType = AnimalType::FLAMINGO; break;
			case 2: animalType = AnimalType::SEAL;     break;
			case 3: animalType = AnimalType::HOESE;    break;
			case 4: animalType = AnimalType::OWL;      break;
			default:animalType = AnimalType::FLAMINGO; break;
			}
		}
		void SetAnimalType(AnimalType a) { animalType = a; }
		std::string WhatKindOfAnimalIsThis() {

			switch (animalType)
			{
			case AnimalType::FLAMINGO:
				return "Flamingo";
				break;
			case AnimalType::HOESE:
				return "Horse";
				break;
			case AnimalType::SEAL:
				return "Seal";
				break;
			case AnimalType::OWL:
				return "Owl";
				break;
			}
			return "I don't know";
		}

		PriorityAnimation GetAnimState()const { return anim; }
		void SetAnimState(PriorityAnimation a) { anim = a; }

		AnimalAttributes();
		~AnimalAttributes() {  }

		virtual bool Deserialize(const rapidjson::Value& obj);

		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		virtual bool ToEditor() override;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

		void SetDefault() {
			 current_health = maxHealth =100.0;
			 current_hunger=maxHunger=100.0;
			 current_sanity=maxSanity=100.0;
			 healthDecayRate=hungerDecayRate=sanityDecayRate=1.0;
			 is_alive = true;
			 isEscaped=false;
		}

	public:
		double current_health;
		double maxHealth;
		double current_hunger;
		double maxHunger;
		double current_sanity;
		double maxSanity;
		double healthDecayRate;
		double hungerDecayRate;
		double sanityDecayRate;
		bool is_alive, isEscaped{ false };

		int healthWarnLevel{ 0 }, hungerWarnLevel{ 0 }, sanityWarnLevel{ 0 };
		int animInnerState{ 0 };

		float timeSinceSanity{ 0.f };

	};
}
