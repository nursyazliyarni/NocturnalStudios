/*!------------------------------------------------------------------------------------------------
    File: ButtonComponent.cpp
    Project: NocturnalStudio

    Brief:
        This file is used to serialize and deserialize items attributes components for ECS.

    Primary Author:
         Ang Yu Xuan (100%)
        - Created the items attributes class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/ButtonComponent.h"
#include "Graphics/Graphics.h"
#include <imgui.h>

namespace NocturnalEngine {

    ButtonComponent::ButtonComponent() {}

    bool ButtonComponent::ToEditor() {
        bool changed = false;
        if (ImGui::CollapsingHeader("ButtonComponent", ImGuiTreeNodeFlags_None)) {

            const char* flagNames[]{
                "GoToScene",        // 1 << 0
                "AudioOnHover",     // 1 << 1
                "AudioOnClick",     // 1 << 2
                "TexChangeOnHover", // 1 << 3
                "TexChangeOnClick", // 1 << 4
                "ScaleOnHover",     // 1 << 5
                "SetGameState",     // 1 << 6
                "ResetStats",       // 1 << 7
                "NextLevel",        // 1 << 8
                "SetCameraZoom",    // 1 << 9
                "MuteAudio",        // 1 << 10
                "AltSceneSwitch",   // 1 << 11
                "ExitGame"          // 1 << 12
            };
            ImGui::SeparatorText("Flags");
            for (int i = 0; i < 13; i++) {
                bool isSet = (flag & (1 << i)) != 0;
                if (ImGui::Checkbox(flagNames[i], &isSet)) {
                    if (isSet) flag |= (1 << i);
                    else       flag &= ~(1 << i);
                    changed = true;
                }

                if (isSet) {
                    ImGui::Indent();
                    switch (i) {
                    case 0: // GoToScene
                    {
                        char buf[256]{};
                        strncpy_s(buf, gotoScene.c_str(), sizeof(buf) - 1);
                        if (ImGui::InputText("Scene Path##goto", buf, sizeof(buf))) {
                            gotoScene = buf;
                            changed = true;
                        }
                        break;
                    }
                    case 1: // AudioOnHover
                        if (ImGui::Checkbox("Random##hoverAudio", &hoverAudioRandom))
                            changed = true;
                        break;
                    case 2: // AudioOnClick
                        if (ImGui::Checkbox("Random##clickAudio", &clickAudioRandom))
                            changed = true;
                        break;
                    case 3: // TexChangeOnHover
                    {

                        if (ImGui::BeginCombo("##hovertex", hoverTexture.c_str()))
                        {
                            for (auto& [str, tex] : ResourceManager::textures)
                            {
                                if (ImGui::Selectable(str.c_str()))
                                {
                                    hoverTexture = str;
                                    //NOCTURNAL_ENGINE_INFO("Loaded {0} Sprite", texturePath.c_str());
                                    changed = true;
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
                            {
                                hoverTexture = *(const std::string*)payload->Data;
                                changed = true;
                            }
                            ImGui::EndDragDropTarget();
                        }
                        break;
                    }
                    case 4: // TexChangeOnClick
                    {
                        if (ImGui::BeginCombo("##clicktex", clickTexture.c_str()))
                        {
                            for (auto& [str, tex] : ResourceManager::textures)
                            {
                                if (ImGui::Selectable(str.c_str()))
                                {
                                    clickTexture = str;
                                    //NOCTURNAL_ENGINE_INFO("Loaded {0} Sprite", texturePath.c_str());
                                    changed = true;
                                }
                            }
                            ImGui::EndCombo();
                        }

                        if (ImGui::BeginDragDropTarget())
                        {
                            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Textures"))
                            {
                                clickTexture = *(const std::string*)payload->Data;
                                changed = true;
                            }
                            ImGui::EndDragDropTarget();
                        }
                        break;
                    }
                    case 5: // ScaleOnHover
                        if (ImGui::DragFloat("Hover Scale##scale", &hoverScale, 0.01f, 0.1f, 10.0f))
                            changed = true;
                        break;
                    case 6: // SetGameState
                    {
                        const char* stateOptions[] = { "STARTING", "PLAYING", "ENDING", "PAUSED" };
                        int current = static_cast<int>(stateToSet);
                        if (ImGui::Combo("Game State##state", &current, stateOptions, 4)) {
                            stateToSet = static_cast<GameManState>(current);
                            changed = true;
                        }
                        break;
                    }
                    case 9: // SetCameraZoom
                        if (ImGui::DragFloat("Camera Zoom##zoom", &cameraZoom, 0.01f, 0.1f, 10.0f))
                            changed = true;
                        break;
                    case 11: // AltSceneSwitch
                    {
                        char buf[256]{};
                        strncpy_s(buf, altSceneSwitch.c_str(), sizeof(buf) - 1);
                        if (ImGui::InputText("Alt Scene##alt", buf, sizeof(buf))) {
                            altSceneSwitch = buf;
                            changed = true;
                        }
                        break;
                    }
                    default: break;
                    }
                    ImGui::Unindent();
                }
            }
        }
        return changed;
    }

    bool ButtonComponent::Deserialize(const rapidjson::Value& obj)
    {
        const char* flagNames[] {
            "GoToSceneFlag",        // 1 << 0
            "AudioOnHoverFlag",     // 1 << 1
            "AudioOnClickFlag",     // 1 << 2
            "TexChangeOnHoverFlag", // 1 << 3
            "TexChangeOnClickFlag", // 1 << 4
            "ScaleOnHoverFlag",     // 1 << 5
            "SetGameStateFlag",     // 1 << 6
            "ResetStatsFlag",       // 1 << 7
            "NextLevelFlag",        // 1 << 8
            "SetCameraZoomFlag",    // 1 << 9
            "MuteAudioFlag",        // 1 << 10
            "AltSceneSwitchFlag",   // 1 << 11
            "ExitGameFlag"          // 1 << 12
        };

        for (int i = 0; i < 13; i++) {
            if (obj.HasMember(flagNames[i])&&obj[flagNames[i]].IsBool()) {

                if (!obj[flagNames[i]].GetBool())continue;
                flag |=(1<<i);
            }
        }

        const char* boolVals[]{
            "AudioOnHoverRandom",     
            "AudioOnClickRandom"
        };
        for (int i = 0; i < 2; i++) {
            if (obj.HasMember(boolVals[i]) && obj[boolVals[i]].IsBool()) {

                switch (i) {
                case 0:
                    hoverAudioRandom = obj[boolVals[i]].GetBool();
                    break;
                case 1:
                    clickAudioRandom = obj[boolVals[i]].GetBool();
                    break;
                }
            }
        }

        const char* stringVals[]{
            "GoToScene",
            "TexChangeOnHover",
            "TexChangeOnClick",
            "AltSceneSwitch",
            "SetGameState"
        };
        for (int i = 0; i < 5; i++)
        {
            if (obj.HasMember(stringVals[i]) && obj[stringVals[i]].IsString()) {
                switch (i)
                {
                case 0:
                    gotoScene = obj[stringVals[i]].GetString();
                    break;
                case 1:
                    hoverTexture = obj[stringVals[i]].GetString();
                    break;
                case 2:
                    clickTexture = obj[stringVals[i]].GetString();
                    break;
                case 3:
                    altSceneSwitch = obj[stringVals[i]].GetString();
                    break;
                case 4:
                    std::string state = obj[stringVals[i]].GetString();
                    std::transform(state.begin(), state.end(), state.begin(), ::toupper);
                    if (state == "STARTING")stateToSet = GameManState::STARTING;
                    if (state == "PLAYING")stateToSet = GameManState::PLAYING;
                    if (state == "ENDING")stateToSet = GameManState::ENDING;
                    if (state == "PAUSED")stateToSet = GameManState::PAUSED;
                    break;
                }
            }
        }

        const char* floatVals[]{
            "ScaleOnHover",
            "SetCameraZoom"
        };
        for (int i = 0; i < 2; i++) {
            if (obj.HasMember(floatVals[i]) && obj[floatVals[i]].IsNumber()) {

                switch (i) {
                case 0:
                    hoverScale = obj[floatVals[i]].GetFloat();
                    break;
                case 1:
                    cameraZoom = obj[floatVals[i]].GetFloat();
                    break;
                }
            }
        }

        return true;
    }

    bool ButtonComponent::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
    

        // GoToScene
        writer->Key("GoToSceneFlag");       writer->Bool((flag & (1 << 0)) != 0);
        writer->Key("GoToScene");           writer->String(gotoScene.c_str());

        // AudioOnHover
        writer->Key("AudioOnHoverFlag");    writer->Bool((flag & (1 << 1)) != 0);
        writer->Key("AudioOnHoverRandom");  writer->Bool(hoverAudioRandom);

        // AudioOnClick
        writer->Key("AudioOnClickFlag");    writer->Bool((flag & (1 << 2)) != 0);
        writer->Key("AudioOnClickRandom");  writer->Bool(clickAudioRandom);

        // TexChangeOnHover
        writer->Key("TexChangeOnHoverFlag"); writer->Bool((flag & (1 << 3)) != 0);
        writer->Key("TexChangeOnHover");    writer->String(hoverTexture.c_str());

        // TexChangeOnClick
        writer->Key("TexChangeOnClickFlag"); writer->Bool((flag & (1 << 4)) != 0);
        writer->Key("TexChangeOnClick");    writer->String(clickTexture.c_str());

        // ScaleOnHover
        writer->Key("ScaleOnHoverFlag");    writer->Bool((flag & (1 << 5)) != 0);
        writer->Key("ScaleOnHover");        writer->Double(static_cast<double>(hoverScale));

        // SetGameState
        writer->Key("SetGameStateFlag");    writer->Bool((flag & (1 << 6)) != 0);
        const char* stateStr = "STARTING";
        if (stateToSet == GameManState::PLAYING) stateStr = "PLAYING";
        else if (stateToSet == GameManState::ENDING)  stateStr = "ENDING";
        else if (stateToSet == GameManState::PAUSED)  stateStr = "PAUSED";
        writer->Key("SetGameState");        writer->String(stateStr);

        // ResetStats
        writer->Key("ResetStatsFlag");      writer->Bool((flag & (1 << 7)) != 0);

        // NextLevel
        writer->Key("NextLevelFlag");       writer->Bool((flag & (1 << 8)) != 0);

        // SetCameraZoom
        writer->Key("SetCameraZoomFlag");   writer->Bool((flag & (1 << 9)) != 0);
        writer->Key("SetCameraZoom");       writer->Double(static_cast<double>(cameraZoom));

        // MuteAudio
        writer->Key("MuteAudioFlag");       writer->Bool((flag & (1 << 10)) != 0);

        // AltSceneSwitch
        writer->Key("AltSceneSwitchFlag");  writer->Bool((flag & (1 << 11)) != 0);
        writer->Key("AltSceneSwitch");      writer->String(altSceneSwitch.c_str());

        // ExitGame
        writer->Key("ExitGameFlag");        writer->Bool((flag & (1 << 12)) != 0);
;
        return true;
    }
}
