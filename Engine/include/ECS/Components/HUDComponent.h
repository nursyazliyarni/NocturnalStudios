/*!------------------------------------------------------------------------------------------------
    File: HUDComponent.h
    Project: NocturnalStudio

	Brief:
		This file contains class and functions used by the HUD component
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the HUDComponent.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "glm/glm.hpp"
namespace NocturnalEngine {

	class HUDComponent : public IComponent
	{
	public:
		enum class ElementType
		{
			Static,
			BossStatic,
			HealthBar,
			HungerBar,
			SanityBar,
			DashBar,
			ChargeAtk,
			Text
		};
		HUDComponent();
		~HUDComponent() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		void ToEditor()override;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }
		ElementType GetType() { return mType; }
		vec2D& GetOffset() { return offset; }
		void SetType(ElementType in) { mType = in; }
		void SetOffset(vec2D in) { offset = in; }
		std::string GetAtk() { return atk; }
		void SetAtk(std::string in) { atk = in; }
		std::string GetDef() { return def; }
		void SetDef(std::string in) { def = in; }
		std::string GetSpd() { return spd; }
		void SetSpd(std::string in) { spd = in; }
		std::string GetCombo() { return combo; }
		void SetCombo(std::string in) { combo = in; }
	private:
		ElementType mType;
		vec2D offset;
		std::string atk;
		std::string def;
		std::string spd;
		std::string combo;
	};
}
