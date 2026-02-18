/*!------------------------------------------------------------------------------------------------
    File: NameTag.h
    Project: NocturnalStudio

	Brief:
 		NameTag Component for tagging each entity with a string name
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the NameTag.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class NameTag : public IComponent
	{
	public:
		NameTag();
		~NameTag() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		void ToEditor()override;

		void SetNameTag(std::string name) { mName = name; }
		std::string& GetNameTag() { return mName; }
		

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:
		std::string mName;
	};


}