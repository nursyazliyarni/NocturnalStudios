/*!------------------------------------------------------------------------------------------------
    File: RigidBody.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize rigid body components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the rigid body class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/RigidBody.h"
#include <imgui.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for RigidBody Component
	****************************************************************************/
	RigidBody::RigidBody() : mAccel{vec2D(1.f,1.f)}, mVelocity{vec2D(1.0f,1.0f)}, mDirection{vec2D(1.0f,0.0f)}, mFriction{0.0f}, mRestitution{0.0f} {}

	bool RigidBody::ToEditor() {
		bool changed = false;
		if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_None))
		{
			//Acceleration
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Acceleration   "); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##AccelerationX", (float*)&mVelocity.x, 0.005f); ImGui::SameLine();
			ImGui::PushID(3);
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##AccelerationY", (float*)&mVelocity.y, 0.005f);
			ImGui::PopID();

			//Direction
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Direction  "); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##DirectionX", (float*)&mDirection.x, 0.005f); ImGui::SameLine();
			ImGui::PushID(4);
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##DirectionY", (float*)&mDirection.y, 0.005f);
			ImGui::PopID();

			//GetFricition
			ImGui::Text("Friction   "); ImGui::SameLine();
			changed |= ImGui::DragFloat("##Friction", (float*)&mFriction, 1.0f);

			//GetRestitution
			ImGui::Text("Restitution"); ImGui::SameLine();
			changed |= ImGui::DragFloat("##Restitution", (float*)&mRestitution, 1.0f);
		}
		return changed;
	}


	/*!*************************************************************************
	Deserialize for RigidBody
	****************************************************************************/
	bool RigidBody::Deserialize(const rapidjson::Value& obj)
	{
		mAccel = vec2D(obj["velX"].GetFloat(), obj["velY"].GetFloat());
		mDirection = vec2D(obj["dirX"].GetFloat(), obj["dirY"].GetFloat());
		mFriction = obj["Friction"].GetFloat();
		mRestitution = obj["Restitution"].GetFloat();
		return true;
	}

	/*!*************************************************************************
	Serialize for RigidBody
	****************************************************************************/
	bool RigidBody::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("velX");
		writer->Double(mAccel.x);
		writer->Key("velY");
		writer->Double(mAccel.y);
		writer->Key("dirX");
		writer->Double(mDirection.x);
		writer->Key("dirY");
		writer->Double(mDirection.y);
		writer->Key("Friction");
		writer->Double(mFriction);
		writer->Key("Restitution");
		writer->Double(mRestitution);
		return true;
	}
	//End RigidBody
}
