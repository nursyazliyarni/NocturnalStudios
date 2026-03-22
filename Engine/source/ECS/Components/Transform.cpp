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
#include "ECS/ParentingSystem.h"
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
			bool isChild = p_Heirarchy.IsChild(entityID);

			// Local space toggle — only show when parented
			if (isChild)
			{
				bool wasLocal = isLocalSpace;
				if (ImGui::Checkbox("Local Space", &isLocalSpace))
				{

					if (!wasLocal && isLocalSpace)
					{
						// Switching TO local — convert world pos to local
						// Local = World - ParentWorld
						ParentPos();
					}
					else if (wasLocal && !isLocalSpace)
					{
						// Switching FROM local — convert local pos to world
						// World = Local + ParentWorld
						UnparentPos();
					}
					changed = true;
				}
				ImGui::SameLine();
				ImGui::TextDisabled("(position relative to parent)");
			}

			// Show label hint for what space we're in
			if (isChild)
			{
				if (isLocalSpace)
					ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "Showing Local Position");
				else
					ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Showing World Position");
			}

			//position
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Position"); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();                    //set a "x" to indicate x-axis
			changed |= ImGui::DragFloat("##PositionX", (float*)&mPosition.x, 0.005f); ImGui::SameLine(); //char name , pass float pointer to position vec2D which hold x and y, the scaling value in imgui
			ImGui::PushID(1);
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##PositionY", (float*)&mPosition.y, 0.005f);
			ImGui::PopID();

			// Show world pos as read-only info when in local space
			if (isChild && isLocalSpace)
			{
				vec2D worldPos = GetWorldPos();
				ImGui::BeginDisabled();
				ImGui::Text("World  "); ImGui::SameLine();
				ImGui::Text("X"); ImGui::SameLine();
				ImGui::DragFloat("##WorldX", &worldPos.x, 0.005f); ImGui::SameLine();
				ImGui::PushID(10);
				ImGui::Text("Y"); ImGui::SameLine();
				ImGui::DragFloat("##WorldY", &worldPos.y, 0.005f);
				ImGui::PopID();
				ImGui::EndDisabled();
			}

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


		
		if (obj.HasMember("posX")&&obj.HasMember("posY")
			&&obj["posX"].IsNumber()&&obj["posY"].IsNumber()) 
		{
			mPosition = vec2D(obj["posX"].GetFloat(), obj["posY"].GetFloat()); }

		if (obj.HasMember("scaleX") && obj.HasMember("scaleY")
			&& obj["scaleX"].IsNumber() && obj["scaleY"].IsNumber())
		{
			mScale = vec2D(obj["scaleX"].GetFloat(), obj["scaleY"].GetFloat());
		}
		
		if(obj.HasMember("Rot")&&obj["Rot"].IsNumber())mRot = obj["Rot"].GetFloat();

		if (obj.HasMember("Position")&&obj["Position"].IsObject())
		{
			mPosition = GetVec2d(obj["Position"]);

		}

		if (obj.HasMember("Scale") && obj["Scale"].IsObject())
		{
			mScale = GetVec2d(obj["Scale"]);

		}

		if (obj.HasMember("Rotation") && obj["Rotation"].IsNumber())mRot = obj["Rotation"].GetFloat();

		if (obj.HasMember("IsLocalSpace") && obj["IsLocalSpace"].IsBool())mRot = obj["IsLocalSpace"].GetBool();

		return true;
	}

	/*!*************************************************************************
	Serialize for Transform Component
	****************************************************************************/
	bool Transform::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		if (p_Heirarchy.IsChild(entityID)) {
			writer->Key("IsLocalSpace");
			writer->Double(isLocalSpace);
		}

		writer->Key("Position");
		writer->StartObject();
		writer->Key("x");
		writer->Double(mPosition.x);
		writer->Key("y");
		writer->Double(mPosition.y);
		writer->EndObject();

		writer->Key("Scale");
		writer->StartObject();
		writer->Key("x");
		writer->Double(mScale.x);
		writer->Key("y");
		writer->Double(mScale.y);
		writer->EndObject();

		writer->Key("Rotation");
		writer->Double(mRot);


		return true;
	}

	vec2D Transform::GetVec2d(const rapidjson::Value& obj) {
		vec2D newPos{ };

		if (obj.HasMember("x") && obj["x"].IsNumber())newPos.x = obj["x"].GetFloat();
		if (obj.HasMember("y") && obj["y"].IsNumber())newPos.y = obj["y"].GetFloat();

		return newPos;
	}


	vec2D Transform::GetWorldPos() {
		if (!p_Heirarchy.IsChild(entityID) || !isLocalSpace) return mPosition;

		Entity parent{ p_Heirarchy.GetParent(entityID) };
		vec2D parentPos{};
		if (p_ecs.HaveComponent<Transform>(parent))parentPos = p_ecs.GetComponent<Transform>(parent).GetPos();

		return parentPos + mPosition;
	}

	void Transform::UnparentPos() {

		Entity parent = p_Heirarchy.GetParent(entityID);
		vec2D parentPos{};
		if (p_ecs.HaveComponent<Transform>(parent))parentPos = p_ecs.GetComponent<Transform>(parent).GetWorldPos();

		// Switching FROM local — convert local pos to world
		// World = Local + ParentWorld
		mPosition = mPosition + parentPos;

		isLocalSpace = false;
	}
	void Transform::ParentPos() {
		Entity parent = p_Heirarchy.GetParent(entityID);
		vec2D parentPos{};
		if (p_ecs.HaveComponent<Transform>(parent))parentPos = p_ecs.GetComponent<Transform>(parent).GetWorldPos();

		// Switching TO local — convert world pos to local
		// Local = World - ParentWorld
		mPosition = mPosition - parentPos;

		isLocalSpace = true;
	}
	//End Transform
}
