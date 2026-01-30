/*!------------------------------------------------------------------------------------------------
    File: AnimalAttributes.h
    Project: NocturnalStudio

	Brief:
		Contains the attributes related to animals in the ECS
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the AnimalAttributes.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "ECS/SceneManager.h"
#include "pch.h"

namespace NocturnalEngine
{
	class AnimalAttributes : public IComponent
	{
	public:
		AnimalAttributes();
		~AnimalAttributes() = default;

		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
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
		bool is_alive;
	};
}