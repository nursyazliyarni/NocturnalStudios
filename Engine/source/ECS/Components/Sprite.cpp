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
		internaltimer(0), LayerOrder(0), SPRITE_type{ texture }
	{
		// Provide one fallback rule so auto mode always has a valid default state.
		AnimationStateRule idle{};
		idle.name = "Idle";
		idle.source = 3; // Always
		idle.compare = 0;
		idle.threshold = 0.0f;
		idle.priority = -100;
		idle.enabled = true;
		stateRules.push_back(idle);
	}

	const char* Sprite::sprite_type_ToString(int enum_sprite_type) {
		switch (enum_sprite_type) {
		case sprite_type::texture: return "texture";
		case sprite_type::shape: return "shape";
		}
	}

	bool Sprite::ToEditor() {
		bool changed = false;
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

			changed |= ImGui::Checkbox("SpriteSheet", &is_SpriteSheet); ImGui::SameLine();
			changed |= ImGui::Checkbox("Animation", &is_Animated);
			
			ImGui::PushItemWidth(100.0f);
			ImGui::Text("Coordinates: "); ImGui::SameLine();
			ImGui::Text("X"); ImGui::SameLine();
			changed |= ImGui::DragInt("##X", (int*)&GetIndex().x, 1); ImGui::SameLine();
			ImGui::Text("Y"); ImGui::SameLine();
			changed |= ImGui::DragInt("##Y", (int*)&GetIndex().y, 1);
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
						changed = true;
					}
				}
				ImGui::EndCombo();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
				{
					texturePath = *(const std::string*)payload->Data;
					changed = true;
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
				changed |= ImGui::DragInt("##SelectedIndex", (int*)&GetIndex().x, 1, 0, GetMaxIndex() - 1);

				ImGui::Text("SetDisplayTime: "); ImGui::SameLine();
				changed |= ImGui::DragFloat("##SetDisplayTime", (float*)&GetDisplayTime()[GetIndex().x], 0.005f, 0.0f);

			}

			if (is_SpriteSheet)
			{
				ImGui::Text("UVCoordinates: "); ImGui::SameLine();
				ImGui::Text("U"); ImGui::SameLine();
				changed |= ImGui::DragFloat("##U", (float*)&GetUVCoor().x, 0.5f); ImGui::SameLine();
				ImGui::PushID(2);
				ImGui::Text("V"); ImGui::SameLine();
				changed |= ImGui::DragFloat("##V", (float*)&GetUVCoor().y, 0.5f);
				ImGui::PopID();
			}
			ImGui::Text("Layering Order: "); ImGui::SameLine();
			// ImGui::DragInt("##layering", (int*)&sprite.LayerOrder, 1, 0, 8);
			changed |= ImGui::DragInt("##layering", (int*)&LayerOrder, 1, 0, NocturnalEngine::Graphic::MAX_LAYERS - 1);

		}
		return changed;
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
		if (obj.HasMember("StateManagerEnabled") && obj["StateManagerEnabled"].IsBool())stateManagerEnabled = obj["StateManagerEnabled"].GetBool();
		if (obj.HasMember("StatePreviewInEditor") && obj["StatePreviewInEditor"].IsBool())statePreviewInEditor = obj["StatePreviewInEditor"].GetBool();
		if (obj.HasMember("StatePreviewMode") && obj["StatePreviewMode"].IsInt())statePreviewMode = obj["StatePreviewMode"].GetInt();

		if (obj.HasMember("AnimationStates") && obj["AnimationStates"].IsArray())
		{
			// Preferred format: dynamic list of user-defined state rules.
			stateRules.clear();
			for (const auto& s : obj["AnimationStates"].GetArray())
			{
				if (!s.IsObject())
					continue;
				AnimationStateRule rule{};
				if (s.HasMember("Name") && s["Name"].IsString()) rule.name = s["Name"].GetString();
				if (s.HasMember("Texture") && s["Texture"].IsString()) rule.textureKey = s["Texture"].GetString();
				if (s.HasMember("Source") && s["Source"].IsInt()) rule.source = s["Source"].GetInt();
				if (s.HasMember("Compare") && s["Compare"].IsInt()) rule.compare = s["Compare"].GetInt();
				if (s.HasMember("Threshold") && s["Threshold"].IsNumber()) rule.threshold = s["Threshold"].GetFloat();
				if (s.HasMember("Priority") && s["Priority"].IsInt()) rule.priority = s["Priority"].GetInt();
				if (s.HasMember("Enabled") && s["Enabled"].IsBool()) rule.enabled = s["Enabled"].GetBool();
				stateRules.push_back(rule);
			}
		}
		else
		{
			// Backward compatibility: migrate old fixed fields if they exist in scene data.
			std::string idleTex{}, hungryTex{}, sickTex{}, angryTex{};
			float hungryThreshold{ 0.5f }, sickThreshold{ 0.5f }, angryThreshold{ 0.5f };
			if (obj.HasMember("StateIdleTexture") && obj["StateIdleTexture"].IsString()) idleTex = obj["StateIdleTexture"].GetString();
			if (obj.HasMember("StateHungryTexture") && obj["StateHungryTexture"].IsString()) hungryTex = obj["StateHungryTexture"].GetString();
			if (obj.HasMember("StateSickTexture") && obj["StateSickTexture"].IsString()) sickTex = obj["StateSickTexture"].GetString();
			if (obj.HasMember("StateAngryTexture") && obj["StateAngryTexture"].IsString()) angryTex = obj["StateAngryTexture"].GetString();
			if (obj.HasMember("StateHungryThreshold") && obj["StateHungryThreshold"].IsNumber()) hungryThreshold = obj["StateHungryThreshold"].GetFloat();
			if (obj.HasMember("StateSickThreshold") && obj["StateSickThreshold"].IsNumber()) sickThreshold = obj["StateSickThreshold"].GetFloat();
			if (obj.HasMember("StateAngryThreshold") && obj["StateAngryThreshold"].IsNumber()) angryThreshold = obj["StateAngryThreshold"].GetFloat();

			if (!idleTex.empty() || !hungryTex.empty() || !sickTex.empty() || !angryTex.empty())
			{
				stateRules.clear();
				AnimationStateRule idle{};
				idle.name = "Idle";
				idle.textureKey = idleTex;
				idle.source = 3;
				idle.compare = 0;
				idle.threshold = 0.0f;
				idle.priority = -100;
				idle.enabled = true;
				stateRules.push_back(idle);

				AnimationStateRule hungry{};
				hungry.name = "Hungry";
				hungry.textureKey = hungryTex;
				hungry.source = 0;
				hungry.compare = 0;
				hungry.threshold = hungryThreshold;
				hungry.priority = 10;
				hungry.enabled = !hungryTex.empty();
				stateRules.push_back(hungry);

				AnimationStateRule sick{};
				sick.name = "Sick";
				sick.textureKey = sickTex;
				sick.source = 1;
				sick.compare = 0;
				sick.threshold = sickThreshold;
				sick.priority = 20;
				sick.enabled = !sickTex.empty();
				stateRules.push_back(sick);

				AnimationStateRule angry{};
				angry.name = "Angry";
				angry.textureKey = angryTex;
				angry.source = 2;
				angry.compare = 0;
				angry.threshold = angryThreshold;
				angry.priority = 30;
				angry.enabled = !angryTex.empty();
				stateRules.push_back(angry);
			}
		}

		if (stateRules.empty())
		{
			// Final safety net when no rules were provided by scene data.
			AnimationStateRule idle{};
			idle.name = "Idle";
			idle.textureKey = mTextureName;
			idle.source = 3;
			idle.compare = 0;
			idle.threshold = 0.0f;
			idle.priority = -100;
			idle.enabled = true;
			stateRules.push_back(idle);
		}
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
		writer->Key("StateManagerEnabled");
		writer->Bool(stateManagerEnabled);
		writer->Key("StatePreviewInEditor");
		writer->Bool(statePreviewInEditor);
		writer->Key("StatePreviewMode");
		writer->Int(statePreviewMode);
		writer->Key("AnimationStates");
		writer->StartArray();
		for (const auto& state : stateRules)
		{
			writer->StartObject();
			writer->Key("Name");
			writer->String(state.name.c_str());
			writer->Key("Texture");
			writer->String(state.textureKey.c_str());
			writer->Key("Source");
			writer->Int(state.source);
			writer->Key("Compare");
			writer->Int(state.compare);
			writer->Key("Threshold");
			writer->Double(state.threshold);
			writer->Key("Priority");
			writer->Int(state.priority);
			writer->Key("Enabled");
			writer->Bool(state.enabled);
			writer->EndObject();
		}
		writer->EndArray();
		return true;
	}
	
	
}
