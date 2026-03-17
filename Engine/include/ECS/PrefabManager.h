/*!------------------------------------------------------------------------------------------------
	File: PrefabManager.h
	Project: NocturnalStudio

	Brief:
		This file contains the class specifications for a prefabmanager. It handles loading prefabs
		from json files and allows users to store prefabs
		

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the PrefabManager class

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS.h"
#include "Serialization/JSONserialization.h"

#define p_PrefabManager NocturnalEngine::PrefabManager::GetInstance()
namespace NocturnalEngine
{
	class PrefabManager:public JSONSerializer {
	public:
		struct PrefabInfo
		{
			std::string prefabName{};
			bool        isDirty{ false };    // has it diverged from the source?
			Entity       prefabID{};          // links instance back to the master prefab
		};

		/*!*************************************************************************
		Singleton
		****************************************************************************/
		static PrefabManager& GetInstance() {
			static PrefabManager instance;
			return instance;
		}

		/*!*************************************************************************
		To check if a prefab exists, used by the prefabmanager and also can be used
		by other things
		****************************************************************************/
		bool DoesPrefabExist(const std::string& name) {
			return prefabNameMap.find(name) != prefabNameMap.end();
		}

		/*!*************************************************************************
		This is a function used to add a created prefab to thr prefabnamemap and the
		list
		****************************************************************************/
		bool AddPrefab(const std::string& prefabName, Entity id) {
			if (DoesPrefabExist(prefabName))return false;

			prefabNameMap.emplace(prefabName, id);
			prefabList.push_back(id);
			return true;
		}

		bool CheckOrAddPrefabFromName(const std::string& name);

		/*!*************************************************************************
		This is a function used by the leveleditor to add a prefab from the inspector
		****************************************************************************/
		bool CloneToList(Entity id, std::string name="");

		/*!*************************************************************************
		This takes a prefab json file and adds it to the prefab list
		****************************************************************************/
		bool MakeFromJson(const std::string& fileName);
		/*!*************************************************************************
		Saves the prefab to a json file
		****************************************************************************/
		bool SaveToJson(const std::string& prefabName,Entity e);
		/*!*************************************************************************
		Deserializes ECS entities and data from JSON file to build ECS (File loading)
		****************************************************************************/
		virtual bool Deserialize(const rapidjson::Value& obj) override;
		/*!*************************************************************************
		Serializes ECS entities and data to JSON file to save ECS (File saving)
		****************************************************************************/
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override;

		/*!*************************************************************************
		Clones Entity from prefab list
		****************************************************************************/
		Entity CloneFromPrefab(const std::string& prefabName);

		bool IsFromPrefab(Entity e) const {
			return prefabRegistry.find(e) != prefabRegistry.end();
		}

		PrefabInfo GetPrefabInfo(Entity e) {
			if (!IsFromPrefab(e)) return {};

			return prefabRegistry[e];
		}

		void ClearRegistry() {
			prefabRegistry.clear();
		}
	private:
		bool SerializeWithName(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const std::string& prefabName,Entity e)const;

		PrefabManager() = default;
		std::vector<Entity> prefabList{};
		std::map<std::string, Entity> prefabNameMap{};

		std::unordered_map<Entity, PrefabInfo> prefabRegistry{};

		std::string prefabToSave{};
		Entity prefabToSaveEntity{ MAX_ENTITIES };
	};
}
