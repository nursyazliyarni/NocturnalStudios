/*!------------------------------------------------------------------------------------------------
    File: Collider.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize collider components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the collider class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Collider.h"
#include <imgui.h>

namespace NocturnalEngine
{
	/*!*************************************************************************
	This is the constructor for the collider component to initialize all data members
	Data members will be changed during the de-serialization function call if the
	specific entity has serialized collider information
	****************************************************************************/
	Collider::Collider() {
		for (int i = 0; i < 2; ++i)
		{
			mColArr[i].mCol = ColliderType::none;
			mColArr[i].mCollisionNormal = vec2D();
			mColArr[i].mMin = vec2D();
			mColArr[i].mMax = vec2D();
			mColArr[i].mOffset = vec2D();
			mColArr[i].mHit = 0;
			mColArr[i].mRadius = 0;
			mColArr[i].is_Alive = true;
		}
	}

	bool Collider::ToEditor() {
        bool changed = false;
        if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_None))
        {
            //selection for collider type
            auto& colliderType = mColArr[0].mCol;
            int colliderIndex = static_cast<int>(colliderType);
            const char* colliderNames = "none\0circle\0bubble\0line\0rect\0box\0bossball\0button";
            ImGui::Text("Collider 1 Type"); ImGui::SameLine();
            changed |= ImGui::Combo("##test", &colliderIndex, colliderNames);
            colliderType = static_cast<Collider::ColliderType>(colliderIndex);

            //positioning the offset
            auto& colliderOffset = mColArr[0].mOffset;
            ImGui::PushItemWidth(100.0f);

            ImGui::Text("OffSet   "); ImGui::SameLine();
            ImGui::Text("X"); ImGui::SameLine();

            changed |= ImGui::DragFloat("##colliderOffsetX", (float*)&colliderOffset.x, 0.05f); ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Text("Y"); ImGui::SameLine();
            changed |= ImGui::DragFloat("##colliderOffsetY", (float*)&colliderOffset.y, 0.05f);
            ImGui::PopID();

            //size of the collider
            if (mColArr[0].mCol == Collider::ColliderType::circle || mColArr[0].mCol == Collider::ColliderType::bubble ||
                mColArr[0].mCol == Collider::ColliderType::bossball)
            {
                auto& colliderSize = mColArr[0].mRadius;
                ImGui::Text("Radius   "); ImGui::SameLine();
                changed |= ImGui::DragFloat("##Radius", (float*)&colliderSize, 0.05f);
            }
            else if (mColArr[0].mCol == Collider::ColliderType::rect || mColArr[0].mCol == Collider::ColliderType::box)
            {
                ImGui::Text("Minimum "); ImGui::SameLine();
                ImGui::Text("X"); ImGui::SameLine();
                changed |= ImGui::DragFloat("##MinimumX", (float*)&mColArr[0].mMin.x, 0.05f); ImGui::SameLine();
                ImGui::Text("Y"); ImGui::SameLine();
                changed |= ImGui::DragFloat("##MinimumY", (float*)&mColArr[0].mMin.y, 0.05f);

                ImGui::Text("Maximum "); ImGui::SameLine();
                ImGui::Text("X"); ImGui::SameLine();
                changed |= ImGui::DragFloat("##MaximumX", (float*)&mColArr[0].mMax.x, 0.05f); ImGui::SameLine();
                ImGui::Text("Y"); ImGui::SameLine();
                changed |= ImGui::DragFloat("##MaximumY", (float*)&mColArr[0].mMax.y, 0.05f);
            }

            //selection for collider 2 type
            auto& colliderType1 = mColArr[1].mCol;
            int colliderIndex1 = static_cast<int>(colliderType1);
            const char* colliderNames1 = "none\0circle\0bubble\0line\0rect\0box\0button";
            ImGui::Text("Collider 2 Type"); ImGui::SameLine();
            changed |= ImGui::Combo("###test", &colliderIndex1, colliderNames1);
            ImGui::PushItemWidth(100.0f);
            colliderType1 = static_cast<Collider::ColliderType>(colliderIndex1);

            //positioning the offset
            auto& colliderOffset1 = mColArr[1].mOffset;

            ImGui::Text("OffSet   "); ImGui::SameLine();
            ImGui::Text("X"); ImGui::SameLine();

            changed |= ImGui::DragFloat("###colliderOffsetX", (float*)&colliderOffset1.x, 0.05f); ImGui::SameLine();
            ImGui::PushID(1);
            ImGui::Text("Y"); ImGui::SameLine();
            changed |= ImGui::DragFloat("###colliderOffsetY", (float*)&colliderOffset1.y, 0.05f);
            ImGui::PopID();

            //size of the collider
            if (mColArr[1].mCol == Collider::ColliderType::circle || mColArr[1].mCol == Collider::ColliderType::bubble ||
                mColArr[1].mCol == Collider::ColliderType::bossball)
            {
                auto& colliderSize1 = mColArr[1].mRadius;
                ImGui::Text("Radius   "); ImGui::SameLine();
                changed |= ImGui::DragFloat("###Radius", (float*)&colliderSize1, 0.05f);
            }
            else if (mColArr[1].mCol == Collider::ColliderType::rect || mColArr[1].mCol == Collider::ColliderType::box)
            {
                ImGui::Text("Minimum "); ImGui::SameLine();
                ImGui::Text("X"); ImGui::SameLine();
                changed |= ImGui::DragFloat("###MinimumX", (float*)&mColArr[1].mMin.x, 0.05f); ImGui::SameLine();
                ImGui::Text("Y"); ImGui::SameLine();
                changed |= ImGui::DragFloat("###MinimumY", (float*)&mColArr[1].mMin.y, 0.05f);

                ImGui::Text("Maximum "); ImGui::SameLine();
                ImGui::Text("X"); ImGui::SameLine();
                changed |= ImGui::DragFloat("###MaximumX", (float*)&mColArr[1].mMax.x, 0.05f); ImGui::SameLine();
                ImGui::Text("Y"); ImGui::SameLine();
                changed |= ImGui::DragFloat("###MaximumY", (float*)&mColArr[1].mMax.y, 0.05f);
            }
        }
        return changed;
	}
	/*!*************************************************************************
	This function de-serializes the level colliders from the given level json file
	****************************************************************************/
	bool Collider::Deserialize(const rapidjson::Value& obj)
	{
		mColArr[0].mCol = static_cast<ColliderType>(obj["ColliderType1"].GetInt());
		mColArr[1].mCol = static_cast<ColliderType>(obj["ColliderType2"].GetInt());
		mColArr[0].mMin = vec2D(obj["minX1"].GetFloat(), obj["minY1"].GetFloat());
		mColArr[1].mMin = vec2D(obj["minX2"].GetFloat(), obj["minY2"].GetFloat());
		mColArr[0].mMax = vec2D(obj["maxX1"].GetFloat(), obj["maxY1"].GetFloat());
		mColArr[1].mMax = vec2D(obj["maxX2"].GetFloat(), obj["maxY2"].GetFloat());
		mColArr[0].mOffset = vec2D(obj["offsetX1"].GetFloat(), obj["offsetY1"].GetFloat());
		mColArr[1].mOffset = vec2D(obj["offsetX2"].GetFloat(), obj["offsetY2"].GetFloat());
		mColArr[0].mRadius = obj["radius1"].GetFloat();
		mColArr[1].mRadius = obj["radius2"].GetFloat();
		return true;
	}
	/*!*************************************************************************
	This function saves the level information by serializing it to the level json 
	file
	****************************************************************************/
	bool Collider::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("ColliderType1");
		writer->Int(static_cast<int>(mColArr[0].mCol));
		writer->Key("ColliderType2");
		writer->Int(static_cast<int>(mColArr[1].mCol));
		writer->Key("minX1");
		writer->Double(mColArr[0].mMin.x);
		writer->Key("minX2");
		writer->Double(mColArr[1].mMin.x);
		writer->Key("minY1");
		writer->Double(mColArr[0].mMin.y);
		writer->Key("minY2");
		writer->Double(mColArr[1].mMin.y);
		writer->Key("maxX1");
		writer->Double(mColArr[0].mMax.x);
		writer->Key("maxX2");
		writer->Double(mColArr[1].mMax.x);
		writer->Key("maxY1");
		writer->Double(mColArr[0].mMax.y);
		writer->Key("maxY2");
		writer->Double(mColArr[1].mMax.y);
		writer->Key("offsetX1");
		writer->Double(mColArr[0].mOffset.x);
		writer->Key("offsetX2");
		writer->Double(mColArr[1].mOffset.x);
		writer->Key("offsetY1");
		writer->Double(mColArr[0].mOffset.y);
		writer->Key("offsetY2");
		writer->Double(mColArr[1].mOffset.y);
		writer->Key("radius1");
		writer->Double(mColArr[0].mRadius);
		writer->Key("radius2");
		writer->Double(mColArr[1].mRadius);
		return true;
	}
}
