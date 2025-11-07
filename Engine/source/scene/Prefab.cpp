/* !------------------------------------------------------------------------------------------------
    File: Prefab.cpp
    Project : NocturnalStudio

    Loads prefab JSON, instantiates entities/components, and spawns scene entities from scenes.json.
    Minimal-diff integration of AI component + safe texture resolution hook.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */

#include "scene/Prefab.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <array>
#include <unordered_map>
#include <functional>

#include "ECS/ECS.h"              // createEntity/addComponent here
#include "ECS/ECS_DEF.h"
#include "ECS/ECS_Component.h"    // Transform, ImageRenderer, BoxCollider, PhysicsComponent, AIComponent, AnimalBehavior
#include "graphics/Camera2D.h"    // Cam2D_RegisterPreset / Cam2D_ActivatePreset

// If you have a global logger elsewhere:
extern Logger logger;

// [0]=ESC, [1..9]=digits
static std::array<SceneID, 10> sKey2Scene{};
static bool sKeymapInit = false;

// sceneName <-> SceneID
static std::unordered_map<std::string, SceneID> sNameToId;
static std::unordered_map<SceneID, std::string> sIdToName;

// Configurable JSON path
static std::string sScenesJsonPath = "assets/scenes/scenes.json";

// Texture lookup hook for "sprite_path" → GL texture id
static std::function<int(const std::string&)> sTextureLookup;

static int ResolveTextureID(const char* p) {
    if (!p || !*p) return 0;
    if (sTextureLookup) return sTextureLookup(std::string{ p });
    return 0; // not wired yet -> invisible sprite until renderer is ready
}

// Call once from init (e.g., Application.cpp) after TextureStore is ready
void Prefab_SetTextureLookup(std::function<int(const std::string&)> fn) {
    sTextureLookup = std::move(fn);
}

static bool LoadScenesJson(rapidjson::Document& d) {
    std::ifstream f(sScenesJsonPath);
    if (!f) return false;
    rapidjson::IStreamWrapper isw(f);
    d.ParseStream(isw);
    return !d.HasParseError();
}

static SceneID NameToId(const char* nm) {
    if (!nm) return SceneID{};
    auto it = sNameToId.find(nm);
    return (it == sNameToId.end()) ? SceneID{} : it->second;
}

// ---- PrefabManager ----
PrefabManager::PrefabManager(ECS& ecs) : entity_component_based_system(ecs) {}

void PrefabManager::Load_Prefab(const std::string& filename_path) {
    std::ifstream objFile(filename_path);
    if (!objFile) {
        logger.error("Failed to open prefab file: " + filename_path);
        return;
    }

    rapidjson::IStreamWrapper obj(objFile);
    auto document = std::make_shared<rapidjson::Document>();
    document->ParseStream(obj);

    if (document->HasParseError() || !document->IsObject()) {
        logger.error("Failed to parse prefab file: " + filename_path);
        return;
    }
    prefab_map[filename_path] = document;
}

EntityID PrefabManager::CreateEntityFromPrefab(
    const std::string& json_path,
    const std::string& entity_type,
    const std::string& object_name)
{
    // 1) Verify prefab file loaded
    auto it = prefab_map.find(json_path);
    if (it == prefab_map.end()) {
        logger.error("Prefab file not loaded: " + json_path);
        return INVALID_ENTITY_ID;
    }

    rapidjson::Document& doc = *(it->second);

    // 2) Expect category layout (e.g., { "animal": { "flamingo": {...} }, "food": {...} })
    if (!doc.HasMember(entity_type.c_str()) || !doc[entity_type.c_str()].IsObject()) {
        logger.error("Prefab category '" + entity_type + "' not found.");
        return INVALID_ENTITY_ID;
    }
    const auto& categoryObj = doc[entity_type.c_str()];

    // 3) Check object name
    if (!categoryObj.HasMember(object_name.c_str()) || !categoryObj[object_name.c_str()].IsObject()) {
        logger.error("Prefab name '" + object_name + "' not found in category '" + entity_type + "'.");
        return INVALID_ENTITY_ID;
    }
    const auto& prefabData = categoryObj[object_name.c_str()];

    // 4) Require components array
    if (!prefabData.HasMember("components") || !prefabData["components"].IsArray()) {
        logger.error("Prefab '" + object_name + "' missing components array.");
        return INVALID_ENTITY_ID;
    }

    // Create entity
    EntityID entity = entity_component_based_system.createEntity(object_name);
    if (entity == INVALID_ENTITY_ID) return INVALID_ENTITY_ID;

    // Add components
    for (const auto& comp : prefabData["components"].GetArray()) {
        AddComponentFromJSON(entity, comp);
    }

    // Optional defaults block (applies to Transform etc.)
    if (prefabData.HasMember("defaults") && prefabData["defaults"].IsObject()) {
        ApplyComponentDefaults(entity, prefabData["defaults"]);
    }

    return entity;
}

bool PrefabManager::PrefabExists(const std::string& prefab_name) const {
    for (const auto& current_pair : prefab_map) {
        const rapidjson::Document& current_document = *current_pair.second;

        // New schema: { "prefabs": { "<name>": {...} } }
        if (current_document.HasMember("prefabs") && current_document["prefabs"].IsObject()) {
            const auto& obj = current_document["prefabs"];
            if (obj.HasMember(prefab_name.c_str())) return true;
        }

        // Old array schema: [ { "name":"...", "components":[...] }, ... ]
        if (current_document.IsArray()) {
            const auto& arr = current_document.GetArray();
            for (const auto& item : arr) {
                if (item.HasMember("name") && item["name"].IsString() &&
                    prefab_name == item["name"].GetString()) {
                    return true;
                }
            }
        }
    }
    return false;
}

EntityID PrefabManager::Instantiate(const std::string& prefab_name, const std::string& entity_name) {
    EntityID id = Instantiate(prefab_name, INVALID_ENTITY_ID);
    if (id != INVALID_ENTITY_ID && !entity_name.empty()) {
        entity_component_based_system.getEntityName(id) = entity_name;
    }
    return id;
}

EntityID PrefabManager::Instantiate(const std::string& prefab_name, EntityID specific_id) {
    for (auto& pair : prefab_map) {
        const rapidjson::Document& current_doc = *pair.second;

        // New schema: { "prefabs": { "<name>": { "components":[...], "defaults":{...} } } }
        if (current_doc.HasMember("prefabs") && current_doc["prefabs"].IsObject()) {
            const auto& prefabs_object = current_doc["prefabs"];
            if (prefabs_object.HasMember(prefab_name.c_str())) {
                const rapidjson::Value& prefab_def = prefabs_object[prefab_name.c_str()];

                EntityID new_entity = entity_component_based_system.createEntity(prefab_name, specific_id);
                if (new_entity == INVALID_ENTITY_ID) return INVALID_ENTITY_ID;

                if (prefab_def.HasMember("components") && prefab_def["components"].IsArray()) {
                    for (const auto& component_value : prefab_def["components"].GetArray()) {
                        AddComponentFromJSON(new_entity, component_value);
                    }
                }
                if (prefab_def.HasMember("defaults") && prefab_def["defaults"].IsObject()) {
                    ApplyComponentDefaults(new_entity, prefab_def["defaults"]);
                }

                return new_entity;
            }
        }

        // Old array schema support
        if (current_doc.IsArray()) {
            const auto& doc_array = current_doc.GetArray();
            for (const auto& array_item : doc_array) {
                if (!array_item.HasMember("name") || !array_item["name"].IsString()) continue;
                if (array_item["name"].GetString() != prefab_name) continue;

                EntityID new_entity = entity_component_based_system.createEntity(prefab_name, specific_id);
                if (new_entity == INVALID_ENTITY_ID) return INVALID_ENTITY_ID;

                if (array_item.HasMember("components") && array_item["components"].IsArray()) {
                    for (const auto& component_value : array_item["components"].GetArray()) {
                        AddComponentFromJSON(new_entity, component_value);
                    }
                }
                if (array_item.HasMember("defaults") && array_item["defaults"].IsObject()) {
                    ApplyComponentDefaults(new_entity, array_item["defaults"]);
                }

                return new_entity;
            }
        }
    }

    logger.warning("Prefab definition not found: " + prefab_name);
    return INVALID_ENTITY_ID;
}

// ---------- Component construction ----------
void PrefabManager::AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json) {
    if (!component_json.HasMember("type") || !component_json["type"].IsString()) return;
    const std::string type = component_json["type"].GetString();

    // Transform
    if (type == "Transform") {
        Transform t{};
        if (component_json.HasMember("x")     && component_json["x"].IsNumber())     t.position.x() = component_json["x"].GetFloat();
        if (component_json.HasMember("y")     && component_json["y"].IsNumber())     t.position.y() = component_json["y"].GetFloat();
        if (component_json.HasMember("w")     && component_json["w"].IsNumber())     t.scale.x()    = component_json["w"].GetFloat();
        if (component_json.HasMember("h")     && component_json["h"].IsNumber())     t.scale.y()    = component_json["h"].GetFloat();
        if (component_json.HasMember("angle") && component_json["angle"].IsNumber()) t.rotation     = component_json["angle"].GetFloat();
        if (component_json.HasMember("height")&& component_json["height"].IsNumber()) t.height      = component_json["height"].GetFloat();
        entity_component_based_system.addComponent<Transform>(entity, t);
        return;
    }

    // SpriteRenderer/ImageRenderer (treat both as ImageRenderer)
    if (type == "SpriteRenderer" || type == "ImageRenderer") {
        ImageRenderer r{};
        r.isVisible = true;
        if (component_json.HasMember("sprite_path") && component_json["sprite_path"].IsString()) {
            r.textureID = ResolveTextureID(component_json["sprite_path"].GetString());
            r.isVisible = (r.textureID > 0);
        }
        entity_component_based_system.addComponent<ImageRenderer>(entity, r);
        return;
    }

    // BoxCollider / AABBCollider
    if (type == "BoxCollider" || type == "AABBCollider") {
        BoxCollider c{};
        if (component_json.HasMember("bounding_box_size") && component_json["bounding_box_size"].IsNumber()) {
            float s = component_json["bounding_box_size"].GetFloat();
            c.width = s; c.height = s;
        }
        if (component_json.HasMember("width")  && component_json["width"].IsNumber())  c.width  = component_json["width"].GetFloat();
        if (component_json.HasMember("height") && component_json["height"].IsNumber()) c.height = component_json["height"].GetFloat();
        entity_component_based_system.addComponent<BoxCollider>(entity, c);
        return;
    }

    // Physics (minimal fields so AI can move)
    if (type == "Physics" || type == "PhysicsComponent") {
        PhysicsComponent pc{};
        if (component_json.HasMember("vx") && component_json["vx"].IsNumber()) pc.velocity.x() = component_json["vx"].GetFloat();
        if (component_json.HasMember("vy") && component_json["vy"].IsNumber()) pc.velocity.y() = component_json["vy"].GetFloat();
        if (component_json.HasMember("mass") && component_json["mass"].IsNumber()) pc.mass = component_json["mass"].GetFloat();
        entity_component_based_system.addComponent<PhysicsComponent>(entity, pc);
        return;
    }

    // AnimalProperties → AnimalBehavior
    if (type == "AnimalProperties" || type == "AnimalBehavior") {
        AnimalBehavior ab{};
        if (component_json.HasMember("health_bar") && component_json["health_bar"].IsNumber()) ab.health_bar = component_json["health_bar"].GetFloat();
        if (component_json.HasMember("hunger_bar") && component_json["hunger_bar"].IsNumber()) ab.hunger_bar = component_json["hunger_bar"].GetFloat();
        if (component_json.HasMember("sanity_bar") && component_json["sanity_bar"].IsNumber()) ab.sanity_bar = component_json["sanity_bar"].GetFloat();
        if (component_json.HasMember("sound") && component_json["sound"].IsString()) ab.sound = component_json["sound"].GetString();
        entity_component_based_system.addComponent<AnimalBehavior>(entity, ab);
        return;
    }

    // AI (minimal-diff integration)
    if (type == "AI") {
        AIComponent ai{};

        if (component_json.HasMember("state") && component_json["state"].IsString()) {
            std::string s = component_json["state"].GetString();
            if      (s == "Chase") ai.state = AIComponent::State::Chase;
            else if (s == "Flee")  ai.state = AIComponent::State::Flee;
            else                   ai.state = AIComponent::State::Wander;
        }
        if (component_json.HasMember("maxSpeed")      && component_json["maxSpeed"].IsNumber())      ai.maxSpeed      = component_json["maxSpeed"].GetFloat();
        if (component_json.HasMember("accel")         && component_json["accel"].IsNumber())         ai.accel         = component_json["accel"].GetFloat();
        if (component_json.HasMember("wanderJitter")  && component_json["wanderJitter"].IsNumber())  ai.wanderJitter  = component_json["wanderJitter"].GetFloat();
        if (component_json.HasMember("chaseStrength") && component_json["chaseStrength"].IsNumber()) ai.chaseStrength = component_json["chaseStrength"].GetFloat();
        if (component_json.HasMember("fleeStrength")  && component_json["fleeStrength"].IsNumber())  ai.fleeStrength  = component_json["fleeStrength"].GetFloat();
        if (component_json.HasMember("avoidRadius")   && component_json["avoidRadius"].IsNumber())   ai.avoidRadius   = component_json["avoidRadius"].GetFloat();
        if (component_json.HasMember("targetName")    && component_json["targetName"].IsString())    ai.targetName    = component_json["targetName"].GetString();
        if (component_json.HasMember("targetWeight")  && component_json["targetWeight"].IsNumber())  ai.targetWeight  = component_json["targetWeight"].GetFloat();

        entity_component_based_system.addComponent<AIComponent>(entity, ai);
        return;
    }

    // Unknown component: log and ignore (safe)
    logger.warning("Unknown component type in JSON: " + type);
}

// Apply flat overrides (x,y,w,h,angle,height) + optional [position:[x,y]]
void PrefabManager::ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults) {
    // Ensure a Transform exists
    Transform* t = entity_component_based_system.getComponent<Transform>(entity);
    if (!t) {
        Transform nt{};
        entity_component_based_system.addComponent<Transform>(entity, nt);
        t = entity_component_based_system.getComponent<Transform>(entity);
    }
    if (!t) return;

    if (defaults.HasMember("x")     && defaults["x"].IsNumber())     t->position.x() = defaults["x"].GetFloat();
    if (defaults.HasMember("y")     && defaults["y"].IsNumber())     t->position.y() = defaults["y"].GetFloat();
    if (defaults.HasMember("w")     && defaults["w"].IsNumber())     t->scale.x()    = defaults["w"].GetFloat();
    if (defaults.HasMember("h")     && defaults["h"].IsNumber())     t->scale.y()    = defaults["h"].GetFloat();
    if (defaults.HasMember("angle") && defaults["angle"].IsNumber()) t->rotation     = defaults["angle"].GetFloat();
    if (defaults.HasMember("height")&& defaults["height"].IsNumber()) t->height      = defaults["height"].GetFloat();

    if (defaults.HasMember("position") && defaults["position"].IsArray()) {
        const auto& a = defaults["position"].GetArray();
        if (a.Size() >= 2) { t->position.x() = a[0].GetFloat(); t->position.y() = a[1].GetFloat(); }
    }
}

// ---- Scenes helpers ----
void PrefabManager::LoadScenes() {
    // Build key→scene map once (rebuilt if SetScenesJsonPath() is called)
    if (sKeymapInit) return;

    rapidjson::Document d;
    if (LoadScenesJson(d)) {
        // Build name→id from in-memory scenes vector ordering
        sNameToId.clear();
        if (d.HasMember("scenes") && d["scenes"].IsObject()) {
            size_t idx = 0;
            for (auto it = d["scenes"].MemberBegin(); it != d["scenes"].MemberEnd(); ++it, ++idx) {
                if (idx < scenes.size()) {
                    sNameToId[it->name.GetString()] = scenes[idx].id;
                    sIdToName[scenes[idx].id] = it->name.GetString();

                    // Per-scene camera preset
                    if (it->value.IsObject() && it->value.HasMember("camera") && it->value["camera"].IsObject()) {
                        const auto& cam = it->value["camera"];
                        Cam2D_Params p;
                        if (cam.HasMember("cx") && cam["cx"].IsNumber())           p.cx = cam["cx"].GetFloat();
                        if (cam.HasMember("cy") && cam["cy"].IsNumber())           p.cy = cam["cy"].GetFloat();
                        if (cam.HasMember("zoom") && cam["zoom"].IsNumber())       p.zoom = cam["zoom"].GetFloat();
                        if (cam.HasMember("minZoom") && cam["minZoom"].IsNumber()) p.minZoom = cam["minZoom"].GetFloat();
                        if (cam.HasMember("maxZoom") && cam["maxZoom"].IsNumber()) p.maxZoom = cam["maxZoom"].GetFloat();
                        Cam2D_RegisterPreset(it->name.GetString(), p);
                    }
                }
            }
        }

        // states.initial / states.keymap
        if (d.HasMember("states") && d["states"].IsObject()) {
            const auto& st = d["states"];
            const char* initialName =
                (st.HasMember("initial") && st["initial"].IsString()) ? st["initial"].GetString() : nullptr;

            if (st.HasMember("keymap") && st["keymap"].IsObject()) {
                const auto& km = st["keymap"];
                for (int i = 1; i <= 9; ++i) {
                    std::string key = std::to_string(i);
                    if (km.HasMember(key.c_str()) && km[key.c_str()].IsString()) {
                        sKey2Scene[static_cast<size_t>(i)] = NameToId(km[key.c_str()].GetString());
                    }
                }
                if (km.HasMember("ESC") && km["ESC"].IsString())
                    sKey2Scene[0] = NameToId(km["ESC"].GetString());
                else if (initialName)
                    sKey2Scene[0] = NameToId(initialName);
            } else if (initialName) {
                sKey2Scene[0] = NameToId(initialName);
            }

            if (initialName) {
                Cam2D_ActivatePreset(initialName);
            }
        }
    }

    // Fallback: map 1..N by order, ESC→first
    bool any = false;
    for (int i = 0; i < 10; ++i) if (sKey2Scene[static_cast<size_t>(i)] != SceneID{}) { any = true; break; }
    if (!any && !scenes.empty()) {
        const size_t n = scenes.size();
        for (size_t i = 1; i <= 9 && i <= n; ++i) sKey2Scene[i] = scenes[i - 1].id;
        sKey2Scene[0] = scenes.front().id;
    }

    sKeymapInit = true;
}

std::string Scenes_GetInitialName() {
    rapidjson::Document d;
    if (!LoadScenesJson(d)) return {};
    if (d.HasMember("states") && d["states"].IsObject()) {
        const auto& st = d["states"];
        if (st.HasMember("initial") && st["initial"].IsString())
            return st["initial"].GetString();
    }
    if (d.HasMember("scenes") && d["scenes"].IsObject()) {
        for (auto it = d["scenes"].MemberBegin(); it != d["scenes"].MemberEnd(); ++it)
            return it->name.GetString();
    }
    return {};
}

SceneID PrefabManager::ResolveSceneKey(int idx) const {
    if (idx < 0 || idx > 9) return SceneID{};
    return sKey2Scene[static_cast<size_t>(idx)];
}

void PrefabManager::SetScenesJsonPath(const std::string& p) {
    sScenesJsonPath = p;
    sKeymapInit = false; // force rebuild next LoadScenes()
}

// Spawn all entities for a scene from scenes.json
bool Prefab_SpawnEntitiesForScene(const std::string& sceneName, PrefabManager& pm) {
    rapidjson::Document d;
    if (!LoadScenesJson(d)) return false;
    if (!d.HasMember("scenes") || !d["scenes"].IsObject()) return false;

    const auto& scenes = d["scenes"];
    if (!scenes.HasMember(sceneName.c_str())) return false;

    const auto& sc = scenes[sceneName.c_str()];
    if (!sc.HasMember("entities") || !sc["entities"].IsArray()) return true;

    ECS& ecs = pm.entity_component_based_system;

    for (const auto& e : sc["entities"].GetArray()) {
        const std::string name   = (e.HasMember("name")   && e["name"].IsString())   ? e["name"].GetString()   : "";
        const std::string prefab = (e.HasMember("prefab") && e["prefab"].IsString()) ? e["prefab"].GetString() : "";
        if (prefab.empty()) continue;

        // Instantiate by prefab name; attach explicit name for ECS lookups
        EntityID id = pm.Instantiate(prefab, name);
        if (id == INVALID_ENTITY_ID) continue;

        // Prefer 'overrides' then 'defaults' for per-instance Transform, etc.
        const rapidjson::Value* ov = nullptr;
        if (e.HasMember("overrides") && e["overrides"].IsObject()) ov = &e["overrides"];
        else if (e.HasMember("defaults") && e["defaults"].IsObject()) ov = &e["defaults"];
        if (ov) pm.ApplyComponentDefaults(id, *ov);

        // If overrides has sprite_path, resolve and apply now
        if (ov && ov->HasMember("sprite_path") && (*ov)["sprite_path"].IsString()) {
            if (auto* r = ecs.getComponent<ImageRenderer>(id)) {
                r->textureID = ResolveTextureID((*ov)["sprite_path"].GetString());
                r->isVisible = (r->textureID > 0);
            }
        }

        // Allow inline extra components per scene entity
        if (e.HasMember("components") && e["components"].IsArray()) {
            for (const auto& comp : e["components"].GetArray()) {
                pm.AddComponentFromJSON(id, comp);
            }
        }
    }
    return true;
}

const std::string& Prefab_GetSceneName(SceneID id) {
    static std::string empty;
    auto it = sIdToName.find(id);
    return (it == sIdToName.end()) ? empty : it->second;
}
