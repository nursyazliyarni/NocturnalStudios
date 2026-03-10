/*!------------------------------------------------------------------------------------------------
    File: Sprite.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize sprite components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the sprite class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Sprite.h"
#include "Animation/Animation.h"
#include "Graphics/Graphics.h"
#include <imgui.h>
namespace NocturnalEngine {

	/*!*************************************************************************
	Ctor for Sprite Component
	****************************************************************************/
	Sprite::Sprite() : mTextureName("Blank"), Index({ 0,0 }), mUVcooridnates({ 512.0f, 512.0f }),
		is_SpriteSheet(false), is_Animated(false), mAlpha(1), MaxIndex_X(0), displayTime(GetMaxIndex()),
		internaltimer(0), LayerOrder(0), SPRITE_type{ texture } {}

	const char* Sprite::sprite_type_ToString(int enum_sprite_type) {
		switch (enum_sprite_type) {
		case sprite_type::texture: return "texture";
		case sprite_type::shape: return "shape";
		}
	}

	void Sprite::ToEditor() {
		if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_None))
		{
			ImGui::Text("Texture/Shape"); ImGui::SameLine();
			if (ImGui::BeginCombo("##Texture/Shape", sprite_type_ToString(SPRITE_type)))
			{
				for (unsigned int i{ 0 }; i < sprite_type::sprite_type_count; i++) {
					sprite_type selected_sprite_type = static_cast<sprite_type>(i);
					bool is_selected = (SPRITE_type == selected_sprite_type);
					if (ImGui::Selectable(sprite_type_ToString(selected_sprite_type), is_selected))
					{
						SPRITE_type = selected_sprite_type;
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Checkbox("SpriteSheet", &is_SpriteSheet); ImGui::SameLine();
			ImGui::Checkbox("Animation", &is_Animated);
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Coordinates: "); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();
			ImGui::DragInt("##X", (int*)&GetIndex().x, 1); ImGui::SameLine();
			ImGui::Text("Y"); ImGui::SameLine();
			ImGui::DragInt("##Y", (int*)&GetIndex().y, 1);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4());
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4());
			ImGui::Text("Image : "); ImGui::SameLine();
			ImGui::PopStyleColor(3);
			auto& texturePath = GetTexture();
			ImGui::SetNextItemWidth(140.0f);

			if (ImGui::BeginCombo("##sprite", texturePath.c_str()))
			{
				for (auto& [str, tex] : ResourceManager::textures)
				{
					if (ImGui::Selectable(str.c_str()))
					{
						texturePath = str;
						NOCTURNAL_ENGINE_INFO("Loaded {0} Sprite", texturePath.c_str());
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
				{
					texturePath = *(const std::string*)payload->Data;
				}
				ImGui::EndDragDropTarget();
			}
			if (is_SpriteSheet)
			{
				GetMaxIndex() = (int)GETTEXTURE(GetTexture())->GetWidth() / 512;
				ImGui::Text("MaxIndex : %d ", GetMaxIndex());
				GetDisplayTime().resize(GetMaxIndex());//resize the number of frames in a sprite  
			}
			if (GetMaxIndex() > 0 && is_Animated)
			{
				ImGui::Text("SelectedIndex: "); ImGui::SameLine();
				ImGui::DragInt("##SelectedIndex", (int*)&GetIndex().x, 1, 0, GetMaxIndex() - 1);

				ImGui::Text("SetDisplayTime: "); ImGui::SameLine();
				ImGui::DragFloat("##SetDisplayTime", (float*)&GetDisplayTime()[GetIndex().x], 0.005f, 0.0f);

			}

			if (is_SpriteSheet)
			{
				ImGui::Text("UVCoordinates: "); ImGui::SameLine();
				ImGui::Text("U"); ImGui::SameLine();
				ImGui::DragFloat("##U", (float*)&GetUVCoor().x, 0.5f); ImGui::SameLine();
				ImGui::PushID(2);
				ImGui::Text("V"); ImGui::SameLine();
				ImGui::DragFloat("##V", (float*)&GetUVCoor().y, 0.5f);
				ImGui::PopID();
			}
			ImGui::Text("Layering Order: "); ImGui::SameLine();
			// ImGui::DragInt("##layering", (int*)&sprite.LayerOrder, 1, 0, 8);
			ImGui::DragInt("##layering", (int*)&LayerOrder, 1, 0, NocturnalEngine::Graphic::MAX_LAYERS - 1);
		}
	}

	/*!*************************************************************************
	Deserialize for Sprite Component
	****************************************************************************/
	bool Sprite::Deserialize(const rapidjson::Value& obj)
	{
		mTextureName = std::string(obj["TextureName"].GetString());
		Index.x = int(obj["Index_X"].GetInt());
		Index.y = int(obj["Index_Y"].GetInt());
		mUVcooridnates = vec2D(obj["Ucoordinates"].GetFloat(), obj["Vcoordinates"].GetFloat());
		is_SpriteSheet = bool(obj["IsSpriteSheet"].GetBool());
		is_Animated = bool(obj["IsAnimated"].GetBool());
		if (obj.HasMember("MaxIndex") && obj["MaxIndex"].IsNumber())MaxIndex_X = int(obj["MaxIndex"].GetInt());
		for (auto i = 0; i < MaxIndex_X; i++)
		{
			std::string DpNum;
			DpNum = "DisplayTime " + std::to_string(i);
			float individualtime = float(obj[DpNum.c_str()].GetFloat());
			displayTime.push_back(individualtime);
		}
		if (is_Animated)
		{
			Animation::spriteContainer.emplace(mTextureName, displayTime);
		}
		LayerOrder= int(obj["LayeringOrder"].GetInt());
		return true;
	}

	/*!*************************************************************************
	Serialize for Sprite Component
	****************************************************************************/
	bool Sprite::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("TextureName");
		writer->String(mTextureName.c_str());
		writer->Key("Index_X");
		writer->Int(Index.x);
		writer->Key("Index_Y");
		writer->Int(Index.y);
		writer->Key("Ucoordinates");
		writer->Double(mUVcooridnates.x);
		writer->Key("Vcoordinates");
		writer->Double(mUVcooridnates.y);
		writer->Key("IsSpriteSheet");
		writer->Bool(is_SpriteSheet);
		writer->Key("IsAnimated");
		writer->Bool(is_Animated);
		writer->Key("MaxIndex");
		writer->Int(MaxIndex_X);
		for (auto i = 0; i < displayTime.size(); i++)
		{
			std::string DpNum;
			DpNum = "DisplayTime " + std::to_string(i);
			writer->Key(DpNum.c_str());
			writer->Double(displayTime[i]);
		}
		writer->Key("LayeringOrder");
		writer->Int(LayerOrder);
		return true;
	}
	
	
}