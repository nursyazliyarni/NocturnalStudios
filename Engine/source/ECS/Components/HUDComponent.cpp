/*!------------------------------------------------------------------------------------------------
    File: HUDComponent.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize HUD components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the HUD component class and its function specification.
		
    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/HUDComponent.h"
#include <imgui.h>

namespace NocturnalEngine {

	/*!*************************************************************************
	Constructor for HUD component
	****************************************************************************/
	HUDComponent::HUDComponent() : mType{ ElementType::Static }, offset{ vec2D() } {}

	void HUDComponent::ToEditor() {
		if (ImGui::CollapsingHeader("HUD", ImGuiTreeNodeFlags_None))
		{
			const char* HUDList = "Static\0Boss Overlay\0Health Bar\0Hunger Bar\0Sanity Bar\0Dash Bar\0Charge Attack\0Text";
			auto HUDType = GetType();
			int HUDIndex = static_cast<int>(HUDType);
			ImGui::Combo("HudType", &HUDIndex, HUDList);
			ImGui::PushItemWidth(100.0f);
			SetType(static_cast<HUDComponent::ElementType>(HUDIndex));

			auto& Offset = GetOffset();
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Offset"); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
			ImGui::DragFloat("##OffsetX", (float*)&Offset.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
			ImGui::PushID(1);
			ImGui::Text("Y"); ImGui::SameLine();
			ImGui::DragFloat("##OffsetY", (float*)&Offset.y, 0.005f);
			ImGui::PopID();
			SetOffset(Offset);
		}
	}

	/*!*************************************************************************
	Derserialize HUD component
	****************************************************************************/
	bool HUDComponent::Deserialize(const rapidjson::Value& obj)
	{
		mType = static_cast<ElementType>(obj["ElementType"].GetInt());
		offset = vec2D(obj["offsetX"].GetFloat(), obj["offsetY"].GetFloat());
		return true;
	}

	/*!*************************************************************************
	Serialize HUD component
	****************************************************************************/
	bool HUDComponent::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("ElementType");
		writer->Int(static_cast<int>(mType));
		writer->Key("offsetX");
		writer->Double(offset.x);
		writer->Key("offsetY");
		writer->Double(offset.y);
		return true;
	}
}