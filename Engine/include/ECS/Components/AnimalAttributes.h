
#pragma once
#include "IComponent.h"
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

	public:
		double health;
		double maxHealth;
		double sanity;
		double maxSanity;
		double hunger;
		double maxHunger;
		double healthDecayRate;
		double sanityDecayRate;
		double hungerDecayRate;
	};
}