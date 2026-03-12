/*!------------------------------------------------------------------------------------------------
	File: Draggable.cpp
	Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize name tag components for ECS.

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the draggable class and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Draggable.h"
#include <imgui.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for NameTag
	****************************************************************************/
	Draggable::Draggable() : isTarget{ false } {}

	bool Draggable::ToEditor() {
		bool changed = false;
		if (ImGui::CollapsingHeader("Draggable", ImGuiTreeNodeFlags_None)) {
			
			changed |= ImGui::Checkbox("Is Target", &IsTarget());
		}
		return changed;
	}

	/*!*************************************************************************
	Deserialize NameTag Component
	****************************************************************************/
	bool Draggable::Deserialize(const rapidjson::Value& obj)
	{
		if (obj.HasMember("isTarget") && obj["isTarget"].IsBool())isTarget = obj["isTarget"].GetBool();
		return true;
	}

	/*!*************************************************************************
	Serialize NameTag Component
	****************************************************************************/
	bool Draggable::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("isTarget");
		writer->Bool(isTarget);
		return true;
	}
}
