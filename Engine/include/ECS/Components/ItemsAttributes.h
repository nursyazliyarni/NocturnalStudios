/*!------------------------------------------------------------------------------------------------
    File: ItemsAttributes.h
    Project: NocturnalStudio

	Brief:
		This file contains the Items Attributes component to all the item related attributes
		used in the ECS
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the ItemsAttributes.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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
		double gain_amount;
		bool is_dragging;
		vec2D duplicate_item_pos;
	};
}