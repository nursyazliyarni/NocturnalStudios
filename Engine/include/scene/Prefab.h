#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "rapidjson/document.h"
#include "ECS/ECS_DEF.h"   // only POD types + EntityID here. No ECS.h in headers.
#include "ECS/ECS_Component.h"

//
struct Prefab_data {
    Transform transform_struct;
    ImageRenderer image_render_struct;
    BoxCollider collider_struct;
    AnimalBehavior animal_behaviour_struct;
};

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
    EntityID CreateEntityFromPrefab(const std::string& json_path, const std::string& entity_type, 
        const std::string& object_name);

    // Create entity from prefab by name
    EntityID Instantiate(const std::string& prefab_name, const std::string& entity_name = "");
    // Create (or reuse) a specific id if your ECS supports it
    EntityID Instantiate(const std::string& prefab_name, EntityID specific_id);

    bool PrefabExists(const std::string& prefab_name) const;

    void LoadScenes();

    SceneID ResolveSceneKey(int idx) const;

    void SetScenesJsonPath(const std::string& path);

    void ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults);
    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);

    ECS& entity_component_based_system;

//private:
//    void ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults);
//    void AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json);

private:
    struct placeholderformove {
        int id{};
    };
    std::vector<placeholderformove> scenes{};
    std::unordered_map<std::string, rapidjson::Document> prefab_map;
    /*ECS& entity_component_based_system;*/
    std::unordered_map<std::string, std::shared_ptr<rapidjson::Document>> prefab_map;
    ECS& entity_component_based_system;
};

// SceneID -> scene name (used by SceneManager to activate camera)
const std::string& Prefab_GetSceneName(SceneID id);

//  scene bootstrap helpers (spawn by scene name) 
bool Prefab_SpawnEntitiesForScene(const std::string& sceneName, PrefabManager& pm);
std::string Scenes_GetInitialName();
