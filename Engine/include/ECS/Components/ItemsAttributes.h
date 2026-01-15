
#pragma once
#include "IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class ItemsAttributes : public IComponent
	{
	public:
		ItemsAttributes();
		~ItemsAttributes() = default;

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
		double healthRestore;
		double healthRegenRate;
		double maxHealthRestore;
		double sanityRestore;
		double sanityRegenRate;
		double maxSanityRestore;
		double hungerRestore;
		double hungerRegenRate;
		double maxHungerRestore;
	};
}