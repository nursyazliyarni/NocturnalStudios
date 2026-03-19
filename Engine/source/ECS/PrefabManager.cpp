/*!------------------------------------------------------------------------------------------------
	File: PrefabManager.cpp
	Project: NocturnalStudio

	Brief:
		This file contains the class specifications for a prefabmanager. It handles loading prefabs
		from json files and allows users to store prefabs

	Primary Author:
		  LAYCOCK ENHUI SARAH (100%)
		- Created the PrefabManager class

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS/PrefabManager.h"
#include "ECS/ParentingSystem.h"
#include "ECS/EngineComponentReg.h"
#include "System/ServiceLocator.h"

namespace NocturnalEngine
{
	bool PrefabManager::CloneToList(Entity id,std::string name) {
		if (!p_ecs.HaveComponent<NameTag>(id))return false;
		if (name.empty())name = p_ecs.GetComponent<NameTag>(id).GetNameTag();

		Entity newEntity{p_Heirarchy.CloneEntity(id,true)};
		bool returnV =AddPrefab(name, newEntity);
		if (returnV)SaveToJson(name, newEntity);
		return returnV;
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

		DeserializePrefabEntity(obj, prefabEntity, prefabName);
		
		return AddPrefab(prefabName, prefabEntity);
	}

	void PrefabManager::DeserializePrefabEntity(const rapidjson::Value& obj, Entity e, const std::string& prefabName)
	{
		// Ensure this entity always has a relationship entry
		// even if it has no parent/children yet
		if (!p_Heirarchy.EntityRelationshipExists(e))
			p_Heirarchy.EnsureRelationshipExists(e, true); // ? add this
		if (!obj.HasMember("Components") || !obj["Components"].IsArray()) return;
		const rapidjson::Value::ConstArray compArr = obj["Components"].GetArray();

		if (compArr.Size() <= 0) // no components
		{
			NameTag nt{};
			nt.SetNameTag(prefabName);
			p_ecs.AddComponent<NameTag>(e, nt);
			return;
		}

		const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>& ecr = p_ECompReg.GetAll();

		ComponentRegistry* gameComp = ServiceLocator::Get().GetGameComponents();
		const std::unordered_map<std::string, ComponentRegistry::ComponentDescriptor>* gcr =
			gameComp
			? &gameComp->GetAll()
			: nullptr;

		for (const rapidjson::Value& comp : compArr)
		{
			if (!comp.HasMember("Component") || !comp["Component"].IsString())continue;

			std::string componentName = comp["Component"].GetString();
			if (ecr.contains(componentName)) {
				ecr.at(componentName).deserializeAndAddFunc(e, comp);
			}
			else if (gcr && gcr->contains(componentName))
			{
				gcr->at(componentName).deserializeAndAddFunc(e, comp);
			}
		}

		if (obj.HasMember("Children") && obj["Children"].IsArray())
		{
			int childNum{ 0 };
			for (const rapidjson::Value& childObj : obj["Children"].GetArray())
			{
				Entity childEntity = p_ecs.CreateEntity(true);
				DeserializePrefabEntity(childObj, childEntity,prefabName+"_Child"+std::to_string(childNum));
				p_Heirarchy.SetParent(childEntity, e, true); 
				childNum++;
			}
		}
	}


	bool PrefabManager::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const{
		return SerializeWithName(writer, prefabToSave,prefabToSaveEntity);
	}

	bool PrefabManager::SerializeWithName(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const std::string& prefabName,Entity e) const{
		writer->StartObject();

		if (prefabName !="")
		{
			writer->Key("Name");
			writer->String(prefabName.c_str());
		}


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

		if (p_Heirarchy.IsParent(e))
		{
			writer->Key("Children");
			writer->StartArray();
			for (Entity child : p_Heirarchy.GetChildren(e))
				SerializeWithName(writer, "", child);
			writer->EndArray();
		}
		writer->EndObject();
		return true;
	}

	Entity PrefabManager::CloneFromPrefab(const std::string& prefabName) {
		if (!DoesPrefabExist(prefabName))return MAX_ENTITIES;

		Entity prefabEntity{ prefabNameMap[prefabName] }, newEntity{ p_Heirarchy.CloneEntity(prefabEntity) };

		PrefabInfo pfInfo{ prefabName,false,prefabEntity };

		prefabRegistry.emplace(newEntity, pfInfo);

		RegisterChildren(prefabEntity, newEntity, prefabName);

		return newEntity;
	}

	bool PrefabManager::CheckOrAddPrefabFromName(const std::string& name) {
		if (DoesPrefabExist(name))return true;

		std::string filename{ "Assets/Prefabs/" + name + ".json" };

		if (!std::filesystem::exists(filename))return false;

		return MakeFromJson(filename);
	}

	void PrefabManager::RegisterChildren(Entity sourceE, Entity newE, const std::string& prefabName) {
		std::vector<Entity> srcChildren = p_Heirarchy.GetChildren(sourceE);
		std::vector<Entity> newChildren = p_Heirarchy.GetChildren(newE);
		auto srcIt = srcChildren.begin();
		auto newIt = newChildren.begin();
		while (srcIt != srcChildren.end() && newIt != newChildren.end())
		{
			// prefabID points to the source child so compareFunc diffs correctly
			prefabRegistry.emplace(*newIt, PrefabInfo{ prefabName, false, *srcIt });
			RegisterChildren(*srcIt, *newIt, prefabName);
			++srcIt; ++newIt;
		}
	}
}
