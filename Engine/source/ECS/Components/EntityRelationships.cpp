/*!------------------------------------------------------------------------------------------------
	File: EntityRelationships.cpp
	Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize HUD components for ECS.

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the HUD component class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/EntityRelationships.h"
#include <imgui.h>

namespace NocturnalEngine {

	/*!*************************************************************************
	Constructor for HUD component
	****************************************************************************/
	Relationship::Relationship() {}

	bool Relationship::ToEditor() {
	
		return true;
	}

	/*!*************************************************************************
	Derserialize HUD component
	****************************************************************************/
	bool Relationship::Deserialize(const rapidjson::Value& obj)
	{
		return true;
	}

	/*!*************************************************************************
	Serialize HUD component
	****************************************************************************/
	bool Relationship::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{

		return true;
	}
}
