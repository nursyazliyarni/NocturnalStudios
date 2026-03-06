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
		is_SpriteSheet(false), is_Animated(false), mAlpha(1), MaxIndex_X(0),displayTime(GetMaxIndex()), internaltimer(0), LayerOrder(0){}


	bool Sprite::ToEditor() {
		bool changed = false;
		if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_None))
		{
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

			if (ImGui::TreeNode("Animation State Manager"))
			{
				changed |= ImGui::Checkbox("Enable State Manager", &stateManagerEnabled);
				if (stateManagerEnabled)
				{
					auto drawStateTexturePicker = [&](const char* label, std::string& textureKey, int id)
					{
						ImGui::PushID(id);
						ImGui::Text("%s", label); ImGui::SameLine();
						ImGui::SetNextItemWidth(180.0f);
						if (ImGui::BeginCombo("##StateTexture", textureKey.c_str()))
						{
							for (auto& [str, tex] : ResourceManager::textures)
							{
								UNREFERENCED_PARAMETER(tex);
								if (ImGui::Selectable(str.c_str()))
								{
									textureKey = str;
									changed = true;
								}
							}
							ImGui::EndCombo();
						}

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
							{
								textureKey = *(const std::string*)payload->Data;
								changed = true;
							}
							ImGui::EndDragDropTarget();
						}
						ImGui::PopID();
					};

					drawStateTexturePicker("Idle Texture", stateIdleTexture, 101);
					drawStateTexturePicker("Hungry Texture", stateHungryTexture, 102);
					drawStateTexturePicker("Sick Texture", stateSickTexture, 103);
					drawStateTexturePicker("Angry Texture", stateAngryTexture, 104);

					changed |= ImGui::SliderFloat("Hungry Threshold", &stateHungryThreshold, 0.0f, 1.0f, "%.2f");
					changed |= ImGui::SliderFloat("Sick Threshold", &stateSickThreshold, 0.0f, 1.0f, "%.2f");
					changed |= ImGui::SliderFloat("Angry Threshold", &stateAngryThreshold, 0.0f, 1.0f, "%.2f");

					changed |= ImGui::Checkbox("Preview In Level Editor", &statePreviewInEditor);
					const char* previewStates[] = { "Auto", "Idle", "Hungry", "Sick", "Angry" };
					changed |= ImGui::Combo("Preview State", &statePreviewMode, previewStates, IM_ARRAYSIZE(previewStates));
				}
				ImGui::TreePop();
			}
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
		if (obj.HasMember("StateIdleTexture") && obj["StateIdleTexture"].IsString())stateIdleTexture = obj["StateIdleTexture"].GetString();
		if (obj.HasMember("StateHungryTexture") && obj["StateHungryTexture"].IsString())stateHungryTexture = obj["StateHungryTexture"].GetString();
		if (obj.HasMember("StateSickTexture") && obj["StateSickTexture"].IsString())stateSickTexture = obj["StateSickTexture"].GetString();
		if (obj.HasMember("StateAngryTexture") && obj["StateAngryTexture"].IsString())stateAngryTexture = obj["StateAngryTexture"].GetString();
		if (obj.HasMember("StateHungryThreshold") && obj["StateHungryThreshold"].IsNumber())stateHungryThreshold = obj["StateHungryThreshold"].GetFloat();
		if (obj.HasMember("StateSickThreshold") && obj["StateSickThreshold"].IsNumber())stateSickThreshold = obj["StateSickThreshold"].GetFloat();
		if (obj.HasMember("StateAngryThreshold") && obj["StateAngryThreshold"].IsNumber())stateAngryThreshold = obj["StateAngryThreshold"].GetFloat();
		if (obj.HasMember("StatePreviewInEditor") && obj["StatePreviewInEditor"].IsBool())statePreviewInEditor = obj["StatePreviewInEditor"].GetBool();
		if (obj.HasMember("StatePreviewMode") && obj["StatePreviewMode"].IsInt())statePreviewMode = obj["StatePreviewMode"].GetInt();
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
		writer->Key("StateIdleTexture");
		writer->String(stateIdleTexture.c_str());
		writer->Key("StateHungryTexture");
		writer->String(stateHungryTexture.c_str());
		writer->Key("StateSickTexture");
		writer->String(stateSickTexture.c_str());
		writer->Key("StateAngryTexture");
		writer->String(stateAngryTexture.c_str());
		writer->Key("StateHungryThreshold");
		writer->Double(stateHungryThreshold);
		writer->Key("StateSickThreshold");
		writer->Double(stateSickThreshold);
		writer->Key("StateAngryThreshold");
		writer->Double(stateAngryThreshold);
		writer->Key("StatePreviewInEditor");
		writer->Bool(statePreviewInEditor);
		writer->Key("StatePreviewMode");
		writer->Int(statePreviewMode);
		return true;
	}
	
	
}
