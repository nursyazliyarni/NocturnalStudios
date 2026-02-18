/*!------------------------------------------------------------------------------------------------
    File: NameTag.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize name tag components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the name tag class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/NameTag.h"
#include <imgui.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for NameTag
	****************************************************************************/
	NameTag::NameTag() : mName{"none"} {}


	void NameTag::ToEditor() {
		if (ImGui::CollapsingHeader("Name", ImGuiTreeNodeFlags_None))
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), mName.c_str());
			if (ImGui::InputText("name", buffer, sizeof(buffer)))
			{
				mName = std::string(buffer);
			}
		}
	}

	/*!*************************************************************************
	Deserialize NameTag Component
	****************************************************************************/
	bool NameTag::Deserialize(const rapidjson::Value& obj)
	{
		mName = obj["NameTag"].GetString();
		return true;
	}

	/*!*************************************************************************
	Serialize NameTag Component
	****************************************************************************/
	bool NameTag::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("NameTag");
		writer->String(mName.c_str());
		return true;
	}
}