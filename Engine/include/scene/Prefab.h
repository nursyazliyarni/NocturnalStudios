
#include "./ECS/ECS.h"
#include <map>

#pragma once
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "rapidjson/document.h"

#pragma once
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include "rapidjson/document.h"
#include "ecs/ECS_DEF.h"

#pragma once
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include "rapidjson/document.h"
#include "ECS/ECS_DEF.h"

// Forward declarations
class BaseComponentPool;
class GameObject;
class ECS;



class PrefabManager {
private:
    std::unordered_map<std::string, rapidjson::Document> prefab_map;
    struct placeholderformove {
        int id{};
    };
    std::vector<placeholderformove> scenes{};
    ECS& entity_component_based_system;

public:
    PrefabManager(ECS& ecs);
    ~PrefabManager() = default;

    void Load_Prefab(const std::string& filename_path);
    EntityID Instantiate(const std::string& prefab_name, const std::string& entity_name = "");
    EntityID Instantiate(const std::string& prefab_name, EntityID specific_id);

    void LoadScenes();

    SceneID ResolveSceneKey(int idx) const;

    void SetScenesJsonPath(const std::string& path);

    // Helper function to check if a prefab exists
    bool PrefabExists(const std::string& prefab_name) const {
        for (const auto& current_pair : prefab_map) {
            const rapidjson::Document& current_document = current_pair.second;
            if (current_document.HasMember("prefabs") && current_document["prefabs"].IsObject()) {
                if (current_document["prefabs"].HasMember(prefab_name.c_str())) {
                    return true;
                }
            }
            if (current_document.IsArray()) {
                const auto& document_array = current_document.GetArray();
                for (const auto& array_item : document_array) {
                    if (array_item.HasMember("name") && array_item["name"].IsString()) {
                        std::string item_name = array_item["name"].GetString();
                        if (item_name == prefab_name) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

private:
    void ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults);
    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);
};