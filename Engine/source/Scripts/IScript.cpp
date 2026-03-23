/*!------------------------------------------------------------------------------------------------
	File: IScript.cpp
	Project: NocturnalStudio

	Brief:
		This file contains the abstract IScript class declaration
		which is responsible for defining the interface for all scripts
		within the engine.

	Primary Author:
		 LAU XIANG YANG (100%)
		- Created the IScript.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "Scripts/IScript.h"
#include <imgui.h>
namespace NocturnalEngine
{
	bool IScript::ToEditor() {
		bool changed{ false };
		for (ScriptField& field : GetFields()) {
			if (auto* p = std::get_if<float*>(&field.ptr))
				changed |= ImGui::DragFloat(field.name.c_str(), *p, 0.1f);
			else if (auto* p = std::get_if<int*>(&field.ptr))
				changed |= ImGui::DragInt(field.name.c_str(), *p);
			else if (auto* p = std::get_if<bool*>(&field.ptr))
				changed |= ImGui::Checkbox(field.name.c_str(), *p);
		}
		return changed;
	}
	 bool IScript::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const {
		 for (const ScriptField& field : const_cast<IScript*>(this)->GetFields()) {
			 writer->Key(field.name.c_str());
			 if (auto* p = std::get_if<float*>(&field.ptr))
				 writer->Double(static_cast<double>(**p));
			 else if (auto* p = std::get_if<int*>(&field.ptr))
				 writer->Int(**p);
			 else if (auto* p = std::get_if<bool*>(&field.ptr))
				 writer->Bool(**p);
		 }
		 return true;
	 }
	 bool IScript::Deserialize(const rapidjson::Value& obj) {
		 for (ScriptField& field : GetFields()) {
			 if (!obj.HasMember(field.name.c_str())) continue;
			 const auto& val = obj[field.name.c_str()];
			 if (auto* p = std::get_if<float*>(&field.ptr))
				 **p = val.GetFloat();
			 else if (auto* p = std::get_if<int*>(&field.ptr))
				 **p = val.GetInt();
			 else if (auto* p = std::get_if<bool*>(&field.ptr))
				 **p = val.GetBool();
		 }
		 return true;
	 }

}
