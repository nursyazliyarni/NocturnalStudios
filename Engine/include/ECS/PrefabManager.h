/*!------------------------------------------------------------------------------------------------
	File: PrefabManager.h
	Project: NocturnalStudio

	Brief:
		

	Primary Author:
		 

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "ECS.h"
#include "Serialization/JSONserialization.h"
namespace NocturnalEngine
{
	class PrefabManager:public JSONSerializer {
	public:
		static PrefabManager& GetInstance() {
			static PrefabManager instance;
			return instance;
		}

		bool DoesPrefabExist(const std::string& name) {
			return prefabNameMap.find(name) != prefabNameMap.end();
		}

		bool AddPrefab(const std::string& prefabName, Entity id) {
			if (DoesPrefabExist(prefabName))return false;

			prefabNameMap.emplace(prefabName, id);
			prefabList.push_back(id);
			return true;
		}

		bool CloneToList(Entity id, std::string name="");

		bool MakeFromJson(const std::string& fileName);

		bool SaveToJson(const std::string& prefabName,Entity e);
		/*!*************************************************************************
		Deserializes ECS entities and data from JSON file to build ECS (File loading)
		****************************************************************************/
		virtual bool Deserialize(const rapidjson::Value& obj) override;
		/*!*************************************************************************
		Serializes ECS entities and data to JSON file to save ECS (File saving)
		****************************************************************************/
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override;
	private:
		bool SerializeWithName(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer, const std::string& prefabName,Entity e)const;

		PrefabManager() = default;
		std::vector<Entity> prefabList{};
		std::map<std::string, Entity> prefabNameMap{};

		std::string prefabToSave{};
		Entity prefabToSaveEntity{ MAX_ENTITIES };
	};
}
