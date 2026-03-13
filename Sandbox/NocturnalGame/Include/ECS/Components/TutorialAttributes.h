/*!------------------------------------------------------------------------------------------------
	File: TutorialAttributes.h
	Project: NocturnalStudio

	Brief:
		This file handles the data needed for TutorialAttribute component for specific entity

	Primary Author:
		 ANG YU XUAN(100%)
		- Created functions to serialize and deserialize the data for for TutorialAttribute component


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "ECS/Components/IComponent.h"
#include "pch.h"

namespace NocturnalEngine
{
	class TutorialAttributes : public IComponent
	{
	public:
		TutorialAttributes();
		~TutorialAttributes() = default;

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

	public:
		int tutorial_order;
	};
}
