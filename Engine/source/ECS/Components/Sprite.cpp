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
		internaltimer(0), LayerOrder(0), SPRITE_type{ texture }, SHAPE_type{rectangle}, circle_radius{}, rgba_color{},
		offset{0.0f},line_len{0.01f},gap_len{0.01f},line_mvspeed{}
	{
		// Provide one fallback rule so auto mode always has a valid default state.
		AnimationStateRule idle{};
		idle.name = "Idle";
		idle.uSize = 512.0f;
		idle.vSize = 512.0f;
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
		default: return "Unknown Sprite Type";
		}
	}

	const char* Sprite::shape_type_ToString(int enum_shape_type) {
		switch (enum_shape_type) {
		case shape_type::rectangle: return "rectangle";
		case shape_type::circle: return "circle";
		default: return "Unknown Shape Type";
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

			if (SPRITE_type == sprite_type::texture) {
				ImGui::PushItemWidth(100.0f);
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

				changed |= ImGui::Checkbox("SpriteSheet", &is_SpriteSheet); ImGui::SameLine();
			}

			else { //shape
				ImGui::Text("Shape Type"); ImGui::SameLine();
				if (ImGui::BeginCombo("##Shape Type", shape_type_ToString(SHAPE_type)))
				{
					for (unsigned int i{ 0 }; i < shape_type::shape_type_count; i++) {
						shape_type selected_shape_type = static_cast<shape_type>(i);
						bool is_selected = (SHAPE_type == selected_shape_type);
						if (ImGui::Selectable(shape_type_ToString(selected_shape_type), is_selected))
						{
							SHAPE_type = selected_shape_type;
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				ImGui::Text("Circle Radius"); ImGui::SameLine();
				ImGui::DragFloat("##Circle Radius", &circle_radius, 1.f, 0.f, 2.f);
				ImGui::Text("RED color"); ImGui::SameLine();
				ImGui::DragFloat("##RED color", &rgba_color[0], 0.05f, 0.f, 1.f);
				ImGui::Text("GREEN color"); ImGui::SameLine();
				ImGui::DragFloat("##GREEN color", &rgba_color[1], 0.05f, 0.f, 1.f);
				ImGui::Text("BLUE color"); ImGui::SameLine();
				ImGui::DragFloat("##BLUE color", &rgba_color[2], 0.05f, 0.f, 1.f);
				ImGui::Text("ALPHA"); ImGui::SameLine();
				ImGui::DragFloat("##ALPHA", &rgba_color[3], 0.05f, 0.f, 1.f);
			}

			
			changed |= ImGui::Checkbox("Animation", &is_Animated);
			
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
		if (obj.HasMember("shape/texture") && obj["shape/texture"].IsInt())
			SPRITE_type = obj["shape/texture"].GetInt();
		if (obj.HasMember("shape type") && obj["shape type"].IsInt())
			SHAPE_type = obj["shape type"].GetInt();
		if (obj.HasMember("circle radius") && obj["circle radius"].IsNumber())
			circle_radius = obj["circle radius"].GetFloat();

		if (obj.HasMember("length of dashed lines") && obj["length of dashed lines"].IsNumber())
			line_len = obj["length of dashed lines"].GetFloat();
		if (obj.HasMember("distance gap") && obj["distance gap"].IsNumber())
			gap_len = obj["distance gap"].GetFloat();
		if (obj.HasMember("line movement speed") && obj["line movement speed"].IsNumber())
			line_mvspeed = obj["line movement speed"].GetFloat();

		if (obj.HasMember("RGBA") && obj["RGBA"].IsArray())
		{
			const auto& arr = obj["RGBA"];
			for (rapidjson::SizeType index{0};index<arr.Size();index++)
			{
				if (arr[index].IsNumber()) rgba_color[index] = arr[index].GetFloat();
			}
		}

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
				rule.uSize = mUVcooridnates.x;
				rule.vSize = mUVcooridnates.y;
				if (s.HasMember("Ucoordinates") && s["Ucoordinates"].IsNumber()) rule.uSize = s["Ucoordinates"].GetFloat();
				if (s.HasMember("Vcoordinates") && s["Vcoordinates"].IsNumber()) rule.vSize = s["Vcoordinates"].GetFloat();
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
				idle.uSize = mUVcooridnates.x;
				idle.vSize = mUVcooridnates.y;
				idle.source = 3;
				idle.compare = 0;
				idle.threshold = 0.0f;
				idle.priority = -100;
				idle.enabled = true;
				stateRules.push_back(idle);

				AnimationStateRule hungry{};
				hungry.name = "Hungry";
				hungry.textureKey = hungryTex;
				hungry.uSize = mUVcooridnates.x;
				hungry.vSize = mUVcooridnates.y;
				hungry.source = 0;
				hungry.compare = 0;
				hungry.threshold = hungryThreshold;
				hungry.priority = 10;
				hungry.enabled = !hungryTex.empty();
				stateRules.push_back(hungry);

				AnimationStateRule sick{};
				sick.name = "Sick";
				sick.textureKey = sickTex;
				sick.uSize = mUVcooridnates.x;
				sick.vSize = mUVcooridnates.y;
				sick.source = 1;
				sick.compare = 0;
				sick.threshold = sickThreshold;
				sick.priority = 20;
				sick.enabled = !sickTex.empty();
				stateRules.push_back(sick);

				AnimationStateRule angry{};
				angry.name = "Angry";
				angry.textureKey = angryTex;
				angry.uSize = mUVcooridnates.x;
				angry.vSize = mUVcooridnates.y;
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
			idle.uSize = mUVcooridnates.x;
			idle.vSize = mUVcooridnates.y;
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
		writer->Key("shape/texture");
		writer->Int(SPRITE_type);
		writer->Key("shape type");
		writer->Int(SHAPE_type);
		writer->Key("circle radius");
		writer->Double(circle_radius);

		writer->Key("length of dashed lines");
		writer->Double(line_len);
		writer->Key("distance gap");
		writer->Double(gap_len);
		writer->Key("line movement speed");
		writer->Double(line_mvspeed);

		writer->Key("RGBA");
		writer->StartArray(); //start array here
		for (const auto& color : rgba_color) {
			writer->Double(color); //serialize array of data here
		}
		writer->EndArray(); //end array here

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
			writer->Key("Ucoordinates");
			writer->Double(state.uSize);
			writer->Key("Vcoordinates");
			writer->Double(state.vSize);
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
