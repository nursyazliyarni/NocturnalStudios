/*!------------------------------------------------------------------------------------------------
    File: TutorialAttributes.cpp
    Project: NocturnalStudio

    Brief:
        This file handles the data needed for TutorialAttribute component for specific entity

    Primary Author:
         ANG YU XUAN(100%)
        - Created functions to serialize and deserialize the data for for TutorialAttribute component


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/TutorialAttributes.h"
#include <imgui.h>

NocturnalEngine::TutorialAttributes::TutorialAttributes() : tutorial_order{  } {}

bool NocturnalEngine::TutorialAttributes::ToEditor() {
    if (ImGui::CollapsingHeader("Tutorial Attributes", ImGuiTreeNodeFlags_None)) {
        ImGui::DragInt("tutorial order", &tutorial_order, 1, 0, 100);
        tutorial_order = std::clamp(tutorial_order, 0, 100);
        return false;
    }
    return true;
}

bool NocturnalEngine::TutorialAttributes::Deserialize(const rapidjson::Value& obj)
{
    if (obj.HasMember("tutorial_order")) tutorial_order = obj["tutorial_order"].GetInt();
    return true;
}

bool NocturnalEngine::TutorialAttributes::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
    writer->Key("tutorial_order");
    writer->Int(tutorial_order);
    return true;
}