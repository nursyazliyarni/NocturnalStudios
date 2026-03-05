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
#include <imgui.h>

namespace NocturnalEngine {

    ButtonComponent::ButtonComponent() {}

    bool ButtonComponent::ToEditor() {
        bool changed = false;
        if (ImGui::CollapsingHeader("ButtonComponent", ImGuiTreeNodeFlags_None)) {
        
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
    
        writer->StartObject();

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

        writer->EndObject();
        return true;
    }
}
