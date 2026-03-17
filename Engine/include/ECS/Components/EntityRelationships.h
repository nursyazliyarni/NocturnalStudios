/*!------------------------------------------------------------------------------------------------
	File: EntityRelationships.h
	Project: NocturnalStudio

	Brief:
		This file contains class and functions used by the HUD component

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the Relationship.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "glm/glm.hpp"
namespace NocturnalEngine {

	class Relationship : public IComponent
	{
	public:

		Relationship();
		~Relationship() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;

		bool ToEditor()override;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:

	};
}
