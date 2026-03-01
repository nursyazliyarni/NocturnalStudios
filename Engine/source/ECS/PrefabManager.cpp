/*!------------------------------------------------------------------------------------------------
	File: PrefabManager.cpp
	Project: NocturnalStudio

	Brief:


	Primary Author:


	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS/PrefabManager.h"
#include "ECS/EngineComponentReg.h"
#include "System/ServiceLocator.h"

namespace NocturnalEngine
{
	bool PrefabManager::CloneToList(Entity id) {

	}

	bool PrefabManager::MakeFromJson(const std::string& fileName) {
		if (!std::filesystem::exists(fileName))return false;
		return DeserializeFromFile(fileName);
	}

	bool PrefabManager::SaveToJson(const std::string& prefabName) {

		prefabToSave = prefabName;
		bool retVal= SerializeToFile("Assets/Prefabs/" + prefabName + ".json");
		prefabToSave = "";
		return retVal;
	}

	bool PrefabManager::Deserialize(const rapidjson::Value& obj) {
		if (!obj.HasMember("Name")||!obj["Name"].IsString())return false;
		if (!obj.HasMember("Components") || !obj["Components"].IsArray()) return false;
		
		std::string prefabName = obj["Name"].GetString();
		if (DoesPrefabExist(prefabName))return false;

		Entity prefabEntity = p_ecs.CreateEntity();

		const rapidjson::Value::ConstArray compArr = obj["Components"].GetArray();
		
		if (compArr.Size() <= 0) // no components
		{
			NameTag nt{};
			nt.SetNameTag(prefabName);
			p_ecs.AddComponent<NameTag>(prefabEntity, nt);
			return AddPrefab(prefabName, prefabEntity);
		}

		const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>& ecr = p_ECompReg.GetAll();

		ComponentRegistry* gameComp = ServiceLocator::Get().GetGameComponents();
		const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>* gcr =
			gameComp
			? &gameComp->GetAll()
			: nullptr;

		for (const rapidjson::Value& comp: compArr)
		{
			if (!comp.HasMember("Component") || !comp["Component"].IsString())continue;

			std::string componentName = comp["Component"].GetString();
			if (ecr.contains(componentName)) {
				ecr.at(componentName).deserializeAndAddFunc(prefabEntity, comp);
			}
			else if (gcr && gcr->contains(componentName))
			{
				gcr->at(componentName).deserializeAndAddFunc(prefabEntity, comp);
			}
		}
		return AddPrefab(prefabName, prefabEntity);
	}

	bool PrefabManager::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const{
		return SerializeWithName(writer, prefabToSave);
	}

	bool PrefabManager::SerializeWithName(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const std::string& prefabName) const{
		return false;
	}

}
