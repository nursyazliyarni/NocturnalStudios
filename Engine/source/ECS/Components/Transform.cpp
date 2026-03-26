/*!------------------------------------------------------------------------------------------------
    File: Transform.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize transform components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the transform class and its function specification.
		
    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Transform.h"
#include <imgui.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for Transform Component
	****************************************************************************/
	Transform::Transform() : mPosition{ vec2D(0.0f,0.0f) }, mScale{ vec2D(1.0f,1.0f) }, mRot{ 0.0f } {}

	bool Transform::ToEditor() {
		bool changed = false;
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_None))
		{
			//position
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Position"); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
			changed |= ImGui::DragFloat("##PositionX", (float*)&mPosition.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
			ImGui::PushID(1);
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##PositionY", (float*)&mPosition.y, 0.005f);
			ImGui::PopID();

			//scale
			ImGui::Text("Scale   "); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
			changed |= ImGui::DragFloat("##Scale", (float*)&mScale.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
			ImGui::PushID(2);
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##Scale", (float*)&mScale.y, 0.005f);
			ImGui::PopID();

			//rotation
			ImGui::Text("Rotation Z"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##", (float*)&mRot, 1.0f);
		}
		return changed;
	}


	/*!*************************************************************************
	Deserialize for Transform Component
	****************************************************************************/
	bool Transform::Deserialize(const rapidjson::Value& obj)
	{
		mPosition = vec2D(obj["posX"].GetFloat(), obj["posY"].GetFloat());
		mScale = vec2D(obj["scaleX"].GetFloat(), obj["scaleY"].GetFloat());
		mRot = obj["Rot"].GetFloat();
		return true;
	}

	/*!*************************************************************************
	Serialize for Transform Component
	****************************************************************************/
	bool Transform::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("posX");
		writer->Double(mPosition.x);
		writer->Key("posY");
		writer->Double(mPosition.y);
		writer->Key("scaleX");
		writer->Double(mScale.x);
		writer->Key("scaleY");
		writer->Double(mScale.y);
		writer->Key("Rot");
		writer->Double(mRot);
		return true;
	}
	//End Transform
}
