/* !------------------------------------------------------------------------------------------------
	File: Prefab.cpp
	Project : NocturnalStudio

	This file contains ...

	Primary Author :
		Name(%)
		- Brief

	Secondary Author :
		Name(%)
		- Brief

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "scene/Prefab.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"        // we call createEntity/addComponent here
#include "ECS/ECS_DEF.h"

#include "graphics/Camera2D.h"   // added (for Cam2D_RegisterPreset / ActivatePreset)

#include "ECS/ECS_Component.h"

// [0]=ESC, [1..9]=digits
static std::array<SceneID, 10> sKey2Scene{};
static bool sKeymapInit = false;

// sceneName -> SceneID (built from JSON "scenes" key order aligned to in-memory scenes vector)
static std::unordered_map<std::string, SceneID> sNameToId;


static std::unordered_map<SceneID, std::string> sIdToName;

// configurable JSON path (fallback stays this)

static std::string sScenesJsonPath = "assets/scenes/scenes.json";




//  Texture lookup hook for sprite_path, GL texture id 
static std::function<int(const std::string&)> sTextureLookup;

// Safe fallback that compiles even if nobody sets sTextureLookup.
static int ResolveTextureID(const char* p) {
    if (!p || !*p) return 0;
    if (sTextureLookup) return sTextureLookup(std::string{ p });
    return 0; // not wired yet -> invisible sprite (isVisible=false)
}

// a simple setter to call once from  init code (e.g., Application.cpp)
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

EntityID PrefabManager::CreateEntityFromPrefab(const std::string& json_path, const std::string& entity_type,
    const std::string& object_name)
{
    //1. Verify prefab file loaded
    auto it = prefab_map.find(json_path);
    if (it == prefab_map.end()) {
        logger.error("Prefab file not loaded: " + json_path);
        return INVALID_ENTITY_ID;
    }

    // Get reference to the loaded document
    rapidjson::Document& doc = *(it->second);

    //2. Check if the category (e.g., "animal", "food") exists
    if (!doc.HasMember(entity_type.c_str()) || !doc[entity_type.c_str()].IsObject()) {
        logger.error("Prefab category '" + entity_type + "' not found.");
        return INVALID_ENTITY_ID;
    }

    const auto& categoryObj = doc[entity_type.c_str()];

    //3. Check if the object (e.g., "flamingo", "seal_food") exists
    if (!categoryObj.HasMember(object_name.c_str()) || !categoryObj[object_name.c_str()].IsObject()) {
        logger.error("Prefab name '" + object_name + "' not found in category '" + entity_type + "'.");
        return INVALID_ENTITY_ID;
    }

    const auto& prefabData = categoryObj[object_name.c_str()];

    //4. Verify that components array exists
    if (!prefabData.HasMember("components") || !prefabData["components"].IsArray()) {
        logger.error("Prefab '" + object_name + "' missing components array.");
        return INVALID_ENTITY_ID;
    }

    // Create entity first
    EntityID entity = entity_component_based_system.createEntity();
    Prefab_data prefab_data;

    // Loop over components
    for (const auto& comp : prefabData["components"].GetArray()) {
        if (!comp.HasMember("type")) continue;
        std::string type = comp["type"].GetString();

        AddComponentFromJSON(entity, comp);
        /*if (type == "Transform") {
            prefab_data.transform_struct.position[0] = comp.HasMember("x") ? comp["x"].GetDouble() : 0.0;
            prefab_data.transform_struct.position[1] = comp.HasMember("y") ? comp["y"].GetDouble() : 0.0;
            prefab_data.transform_struct.scale[0] = comp.HasMember("w") ? comp["w"].GetDouble() : 1.0; // safe default
            prefab_data.transform_struct.scale[1] = comp.HasMember("h") ? comp["h"].GetDouble() : 1.0;
            prefab_data.transform_struct.rotation = comp.HasMember("angle") ? comp["angle"].GetDouble() : 0.0;
            entity_component_based_system.addComponent<Transform>(entity, prefab_data.transform_struct);
        }

        else if (type == "SpriteRenderer") {
            prefab_data.image_render_struct.image_path = comp["sprite_path"].GetString();
            entity_component_based_system.addComponent<ImageRenderer>(entity, prefab_data.image_render_struct);
        }

        else if (type == "AABBCollider") {
            prefab_data.collider_struct.width = comp["bounding_box_size"].GetDouble();
            prefab_data.collider_struct.height = comp["bounding_box_size"].GetDouble();
            entity_component_based_system.addComponent<BoxCollider>(entity, prefab_data.collider_struct);
        }

        else if (type == "AnimalProperties") {
            prefab_data.animal_behaviour_struct.health_bar = comp["health_bar"].GetDouble();
            prefab_data.animal_behaviour_struct.hunger_bar = comp["hunger_bar"].GetDouble();
            prefab_data.animal_behaviour_struct.sanity_bar = comp["sanity_bar"].GetDouble();
            entity_component_based_system.addComponent<AnimalBehavior>(entity, prefab_data.animal_behaviour_struct);
        }

        else {
            logger.warning("Unknown component type: " + type);
        }*/
    }
    return entity;
}


bool PrefabManager::PrefabExists(const std::string& prefab_name) const {
    for (const auto& current_pair : prefab_map) {
        const rapidjson::Document& current_document = *current_pair.second;

        if (current_document.HasMember("prefabs") && current_document["prefabs"].IsObject()) {
            const auto& obj = current_document["prefabs"];
            if (obj.HasMember(prefab_name.c_str())) return true;
        }

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


//EntityID PrefabManager::Instantiate(const std::string& prefab_name, const std::string& entity_name) {
//	UNREFERENCED_PARAMETER(entity_name);
//
//    return Instantiate(prefab_name, INVALID_ENTITY_ID);
//}

EntityID PrefabManager::Instantiate(const std::string& prefab_name, const std::string& entity_name) {
    // Reuse the existing instantiate path
    EntityID id = Instantiate(prefab_name, INVALID_ENTITY_ID);
    if (id != INVALID_ENTITY_ID && !entity_name.empty()) {
        // Rename the freshly created entity so findEntityByName works
        entity_component_based_system.getEntityName(id) = entity_name;
    }
    return id;
}


EntityID PrefabManager::Instantiate(const std::string& prefab_name, EntityID specific_id) {
    // Find the prefab document
    for (auto& pair : prefab_map) {
        const rapidjson::Document& current_doc = *pair.second;

        // Handle new schema: { "prefabs": { "<name>": { "components":[...], "defaults":{...} }, ... } }
        if (current_doc.HasMember("prefabs") && current_doc["prefabs"].IsObject()) {
            const auto& prefabs_object = current_doc["prefabs"];
            if (prefabs_object.HasMember(prefab_name.c_str())) {
                const rapidjson::Value& prefab_definition = prefabs_object[prefab_name.c_str()];

                // Create entity
                EntityID new_entity = entity_component_based_system.createEntity(prefab_name, specific_id);
                if (new_entity == INVALID_ENTITY_ID) {
                    return INVALID_ENTITY_ID;
                }

                // Add components
                if (prefab_definition.HasMember("components") && prefab_definition["components"].IsArray()) {
                    const auto& components_array = prefab_definition["components"].GetArray();
                    for (const auto& component_value : components_array) {
                        AddComponentFromJSON(new_entity, component_value);
                    }
                }

                // Apply defaults if any
                if (prefab_definition.HasMember("defaults") && prefab_definition["defaults"].IsObject()) {
                    ApplyComponentDefaults(new_entity, prefab_definition["defaults"]);
                }

                return new_entity;
            }
        }

        // Handle old array schema
        if (current_doc.IsArray()) {
            const auto& doc_array = current_doc.GetArray();
            for (const auto& array_item : doc_array) {
                if (!array_item.HasMember("name") || !array_item["name"].IsString()) continue;

                std::string item_name = array_item["name"].GetString();
                if (item_name == prefab_name) {
                    EntityID new_entity = entity_component_based_system.createEntity(item_name, specific_id);
                    if (new_entity == INVALID_ENTITY_ID) {
                        return INVALID_ENTITY_ID;
                    }

                    if (array_item.HasMember("components") && array_item["components"].IsArray()) {
                        const auto& components_array = array_item["components"].GetArray();
                        for (const auto& component_value : components_array) {
                            AddComponentFromJSON(new_entity, component_value);
                        }
                    }

                    return new_entity;
                }
            }
        }
    }

    std::cout << "Prefab definition not found: " << prefab_name << "\n";
    return INVALID_ENTITY_ID;
}

void PrefabManager::AddComponentFromJSON(EntityID entity, const rapidjson::Value& component_json) {
    if (!component_json.HasMember("type") || !component_json["type"].IsString()) {
        return;
    }
    std::string component_type = component_json["type"].GetString();
    
    std::stringstream msg;
    msg<<"Adding Component " << component_type << " to Entity " << entity;
    logger.log(msg.str());
    if (ComponentFactory::CreateFromName(entity,component_type, component_json))
    {
        msg.str("");
        msg.clear();
        msg << component_type << " succsessfully added to Entity " << entity;
        logger.log(msg.str());
    }
    
}

void PrefabManager::LoadScenes() {
    // Build key?scene map once (rebuilt if SetScenesJsonPath is called)
    if (!sKeymapInit) {
        rapidjson::Document d;
        if (LoadScenesJson(d)) {
            // Build name?id WITHOUT referencing the 'Scene' type in a free function.
            sNameToId.clear();
            if (d.HasMember("scenes") && d["scenes"].IsObject()) {
                size_t idx = 0;
                for (auto it = d["scenes"].MemberBegin(); it != d["scenes"].MemberEnd(); ++it, ++idx) {
                    if (idx < scenes.size()) {
                        // scenes[idx] is your member vector; just read its id
                        sNameToId[it->name.GetString()] = scenes[idx].id;


                        sIdToName[scenes[idx].id] = it->name.GetString(); 

                        //  register per-scene camera if present
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

                    // digits 1..9
                    for (int i = 1; i <= 9; ++i) {
                        std::string key = std::to_string(i);
                        if (km.HasMember(key.c_str()) && km[key.c_str()].IsString()) {
                            sKey2Scene[static_cast<size_t>(i)] = NameToId(km[key.c_str()].GetString());
                        }
                    }

                    // ESC
                    if (km.HasMember("ESC") && km["ESC"].IsString())
                        sKey2Scene[0] = NameToId(km["ESC"].GetString());
                    else if (initialName)
                        sKey2Scene[0] = NameToId(initialName);
                }
                else if (initialName) {
                    // no keymap ? still map ESC to initial
                    sKey2Scene[0] = NameToId(initialName);
                }

                // activate initial scene camera 
                if (initialName) {
                    Cam2D_ActivatePreset(initialName);
                }

            }
        }

        // Fallback: nothing mapped ? 1..N from in-memory order, ESC?first
        bool any = false;
        for (int i = 0; i < 10; ++i) {
            if (sKey2Scene[static_cast<size_t>(i)] != SceneID{}) {
                any = true;
                break;
            }
        }
        if (!any && !scenes.empty()) {
            const size_t n = scenes.size();
            for (size_t i = 1; i <= 9 && i <= n; ++i)
                sKey2Scene[i] = scenes[i - 1].id;
            sKey2Scene[0] = scenes.front().id;
        }

        sKeymapInit = true;
    }
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
    sKeymapInit = false; // force rebuild next Update()
}

void PrefabManager::ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults) {
    // Ensure a Transform exists
    Transform* t = entity_component_based_system.getComponent<Transform>(entity);
    if (!t) {
        Transform nt{};
        entity_component_based_system.addComponent<Transform>(entity, nt);
        t = entity_component_based_system.getComponent<Transform>(entity);
    }
    if (!t) return; // fail-safe

    // Apply flat defaults (x,y,w,h,angle,height)
    if (defaults.HasMember("x") && defaults["x"].IsNumber())      t->position.x() = defaults["x"].GetFloat();
    if (defaults.HasMember("y") && defaults["y"].IsNumber())      t->position.y() = defaults["y"].GetFloat();
    if (defaults.HasMember("w") && defaults["w"].IsNumber())      t->scale.x() = defaults["w"].GetFloat();
    if (defaults.HasMember("h") && defaults["h"].IsNumber())      t->scale.y() = defaults["h"].GetFloat();
    if (defaults.HasMember("angle") && defaults["angle"].IsNumber())  t->rotation = defaults["angle"].GetFloat();
    if (defaults.HasMember("height") && defaults["height"].IsNumber()) t->height = defaults["height"].GetFloat();

    // Optional: array-style position override
    if (defaults.HasMember("position") && defaults["position"].IsArray()) {
        const auto& a = defaults["position"].GetArray();
        if (a.Size() >= 2) { t->position.x() = a[0].GetFloat(); t->position.y() = a[1].GetFloat(); }
    }
}


const std::string& Prefab_GetSceneName(SceneID id) {
    static std::string empty;
    auto it = sIdToName.find(id);
    return (it == sIdToName.end()) ? empty : it->second;
}
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
        std::string name = (e.HasMember("name") && e["name"].IsString()) ? e["name"].GetString() : "";
        std::string prefab = (e.HasMember("prefab") && e["prefab"].IsString()) ? e["prefab"].GetString() : "";
        if (prefab.empty()) continue;

        EntityID id = pm.Instantiate(prefab, name);
        if (id == INVALID_ENTITY_ID) continue;

        // Prefer 'overrides' but accept 'defaults'
        const rapidjson::Value* ov = nullptr;
        if (e.HasMember("overrides") && e["overrides"].IsObject()) ov = &e["overrides"];
        else if (e.HasMember("defaults") && e["defaults"].IsObject()) ov = &e["defaults"];

        if (ov) pm.ApplyComponentDefaults(id, *ov);

        // If overrides has sprite_path, resolve and apply to ImageRenderer now
        if (ov && ov->HasMember("sprite_path") && (*ov)["sprite_path"].IsString()) {
            if (auto* r = ecs.getComponent<ImageRenderer>(id)) {
                r->textureID = ResolveTextureID((*ov)["sprite_path"].GetString());
                r->isVisible = (r->textureID > 0);
            }
        }

        // Optional: allow per-entity extra components inline
        if (e.HasMember("components") && e["components"].IsArray()) {
            for (const auto& comp : e["components"].GetArray())
                pm.AddComponentFromJSON(id, comp);
        }
    }
    return true;
}
