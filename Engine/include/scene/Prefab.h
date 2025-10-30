#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "rapidjson/document.h"
#include "ECS/ECS_DEF.h"   // only POD types + EntityID here. No ECS.h in headers.

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
    void     Load_Prefab(const std::string& filename_path);

    // Create entity from prefab by name
    EntityID Instantiate(const std::string& prefab_name, const std::string& entity_name = "");
    // Create (or reuse) a specific id if your ECS supports it
    EntityID Instantiate(const std::string& prefab_name, EntityID specific_id);

    bool PrefabExists(const std::string& prefab_name) const;

private:
    void ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults);
    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);

private:
    std::unordered_map<std::string, rapidjson::Document> prefab_map;
    ECS& entity_component_based_system;
};
