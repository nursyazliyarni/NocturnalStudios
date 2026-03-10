#include "pch.h"
#include "ECS/Components/TutorialAttributes.h"
#include <imgui.h>

NocturnalEngine::TutorialAttributes::TutorialAttributes() : tutorial_order{  } {}

bool NocturnalEngine::TutorialAttributes::ToEditor() {
    if (ImGui::CollapsingHeader("Level Attributes", ImGuiTreeNodeFlags_None)) {
        ImGui::DragInt("tutorial order", &tutorial_order, 1, 0, 100);
        return false;
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