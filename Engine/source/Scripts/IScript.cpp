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
			else if (auto* p = std::get_if<std::string*>(&field.ptr)) {
				static std::unordered_map<std::string, std::array<char, 256>> buffers;

				// Only initialize the buffer if it doesn't exist yet
				if (buffers.find(field.name) == buffers.end()) {
					buffers[field.name] = {};
					strncpy_s(buffers[field.name].data(), 256, (*p)->c_str(), 255);
				}

				// Sync buffer from source when not being edited
				if (!ImGui::IsItemActive()) {
					strncpy_s(buffers[field.name].data(), 256, (*p)->c_str(), 255);
				}

				ImGui::InputText(field.name.c_str(), buffers[field.name].data(), 256);
				if (ImGui::IsItemDeactivatedAfterEdit()) {
					**p = buffers[field.name].data();
					changed = true;
				}
			}
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
			 else if (auto* p = std::get_if<std::string*>(&field.ptr))
				 writer->String((*p)->c_str());
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
			 else if (auto* p = std::get_if<std::string*>(&field.ptr))
				 **p = val.GetString();
		 }
		 return true;
	 }

}
