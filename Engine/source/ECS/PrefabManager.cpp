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
	bool PrefabManager::CloneToList(Entity id,std::string name) {
		if (!p_ecs.HaveComponent<NameTag>(id))return false;
		if (name.empty())name = p_ecs.GetComponent<NameTag>(id).GetNameTag();

		Entity newEntity{ p_ecs.CloneEntity(id,true) };
		return AddPrefab(name, newEntity);
	}

	bool PrefabManager::MakeFromJson(const std::string& fileName) {
		if (!std::filesystem::exists(fileName))return false;
		return DeserializeFromFile(fileName);
	}

	bool PrefabManager::SaveToJson(const std::string& prefabName,Entity e) {

		prefabToSave = prefabName;
		prefabToSaveEntity = e;
		if (!DoesPrefabExist(prefabName)) return false;
		bool retVal= SerializeToFile("Assets/Prefabs/" + prefabName + ".json");
		prefabToSave = "";
		prefabToSaveEntity = MAX_ENTITIES;
		return retVal;
	}

	bool PrefabManager::Deserialize(const rapidjson::Value& obj) {
		if (!obj.HasMember("Name")||!obj["Name"].IsString())return false;
		if (!obj.HasMember("Components") || !obj["Components"].IsArray()) return false;
		
		std::string prefabName = obj["Name"].GetString();
		if (DoesPrefabExist(prefabName))return false;

		Entity prefabEntity = p_ecs.CreateEntity(true);

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
		return SerializeWithName(writer, prefabToSave,prefabToSaveEntity);
	}

	bool PrefabManager::SerializeWithName(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const std::string& prefabName,Entity e) const{
		writer->StartObject();
		writer->Key("Name");
		writer->String(prefabName.c_str());

		writer->Key("Components");
		writer->StartArray();
		//Engine components
		for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> comp : p_ECompReg.GetAll())
		{
			if (comp.second.hasFunc(e))
			{
				writer->StartObject();
				//component name always comes first,
				writer->Key("Component");
				writer->String(comp.first.c_str());
				// then start writing down the component
				comp.second.serializeFunc(e, writer);
				writer->EndObject();

			}
		}

		//Game components
		if (ComponentRegistry* meow = ServiceLocator::Get().GetGameComponents())
		{
			for (std::pair<const std::string, const ComponentRegistry::ComponentDescriptor> comp : meow->GetAll())
			{
				if (comp.second.hasFunc(e))
				{
					writer->StartObject();
					writer->Key("Component");
					writer->String(comp.first.c_str());
					comp.second.serializeFunc(e, writer);
					writer->EndObject();

				}
			}
		}
		writer->EndArray();
		writer->EndObject();
		return true;
	}

}
