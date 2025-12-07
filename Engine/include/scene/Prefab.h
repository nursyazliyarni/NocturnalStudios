/* !------------------------------------------------------------------------------------------------
    File: Prefab.h
    Project : NocturnalStudio

    This file contains ...

     Author :
        Name(%)
        - Brief

     Author :
        Name(%) Rishika Bimal Attodi
        - Integrated with prefabs.json

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */
#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "rapidjson/document.h"
#include "ECS/ECS_DEF.h"   // only POD types + EntityID here. No ECS.h in headers.
#include "ECS/ECS_Component.h"



#define UNREFERENCED_PARAMETER(P) (void)(P)

// Forward declarations only (keep header light)
class ECS;

class PrefabManager {
public:
    explicit PrefabManager(ECS& ecs);
    ~PrefabManager() = default;

    // Load one JSON file containing either:
    //  A) { "prefabs": { "<name>": { "components":[...], "defaults":{...} } } }
    //  or
    //  B) [ { "name":"...", "components":[...] }, ... ]
    void Load_Prefab(const std::string& filename_path);

    EntityID CreateEntityFromPrefab(const std::string& prefab_name, const std::string& object_name = "");

    bool PrefabExists(const std::string& prefab_name) const;

    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);

    bool IsIDPrefab(EntityID id);

//private:
//    void ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults);
//    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);

private:
    ECS& entity_component_based_system;
    struct placeholderformove {
        int id{};
    };
    std::vector<placeholderformove> scenes{};
    std::unordered_map<std::string, EntityID> prefab_map{};
};

// SceneID -> scene name (used by SceneManager to activate camera)
//const std::string& Prefab_GetSceneName(SceneID id);

std::string Scenes_GetInitialName();
