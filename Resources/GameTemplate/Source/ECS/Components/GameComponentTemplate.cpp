/*!------------------------------------------------------------------------------------------------
    File: GameComponentTemplate.cpp
    Project: TEMPLATE_PROJECT_NAME

	Brief:
		This file is used to serialize and deserialize animal attribute components for ECS.
	
    Primary Author:
         

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/GameComponentTemplate.h"
#include <imgui.h>
NocturnalEngine::GameComponentTemplate::GameComponentTemplate()  {}

bool NocturnalEngine::GameComponentTemplate::ToEditor() {
    if (ImGui::CollapsingHeader("GameComponentTemplate", ImGuiTreeNodeFlags_None)) {
        
    }
    return false;
}


bool NocturnalEngine::GameComponentTemplate::Deserialize(const rapidjson::Value& obj)
{
	UNREFERENCED_PARAMETER(obj);
	return true;
}

bool NocturnalEngine::GameComponentTemplate::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
    UNREFERENCED_PARAMETER(writer);
	return true;
}
