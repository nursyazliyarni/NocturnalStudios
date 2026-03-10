#include "pch.h"
#include "ECS/Components/Tutorial_Attributes.h"
#include <imgui.h>

NocturnalEngine::TutorialAttributes::TutorialAttributes() : tutorial_order{  } {}

void NocturnalEngine::TutorialAttributes::ToEditor() {
    if (ImGui::CollapsingHeader("Level Attributes", ImGuiTreeNodeFlags_None)) {
        ImGui::DragInt("tutorial order", &tutorial_order, 1, 0, 100);
    }
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