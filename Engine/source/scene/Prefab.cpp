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

#include "ECS/ECS_Component.h"
#include "ECS/ECS.h"        // we call createEntity/addComponent here
#include "ECS/ECS_DEF.h"

#include "graphics/Camera2D.h"   // added (for Cam2D_RegisterPreset / ActivatePreset)



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
        std::cout << "Failed to open prefab file: " << filename_path << "\n";
        return;
    }

    rapidjson::IStreamWrapper obj(objFile);
    rapidjson::Document document;
    document.ParseStream(obj);

    if (document.HasParseError() || !document.IsObject()) {
        std::cout << "Failed to parse prefab file: " << filename_path << "\n";
        return;
    }

    prefab_map[filename_path] = std::move(document);
    std::cout << "Loaded prefab: " << filename_path << "\n";
}


bool PrefabManager::PrefabExists(const std::string& prefab_name) const {
    for (const auto& current_pair : prefab_map) {
        const rapidjson::Document& current_document = current_pair.second;

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


EntityID PrefabManager::Instantiate(const std::string& prefab_name, const std::string& entity_name) {
	UNREFERENCED_PARAMETER(entity_name);

    return Instantiate(prefab_name, INVALID_ENTITY_ID);
}

EntityID PrefabManager::Instantiate(const std::string& prefab_name, EntityID specific_id) {
    // Find the prefab document
    for (auto& pair : prefab_map) {
        const rapidjson::Document& current_doc = pair.second;

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
    std::cout << "Adding Component "<< component_type<<" to Entity "<<entity<<"\n";
    if (entity_component_based_system.addComponent(entity, ComponentFactory::Create(component_type, component_json)))
    {
        std::cout << component_type << " succsessfully added to Entity " << entity << "\n";
    }
    
}

//void PrefabManager::ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults) {
//    // This function can be used to apply runtime overrides to components
//    // For example, setting specific positions or properties when instantiating
//    if (Transform* t = entity_component_based_system.getComponent<Transform>(entity)) {
//        if (defaults.HasMember("x") && defaults["x"].IsNumber()) t->position.x() = defaults["x"].GetFloat();
//        if (defaults.HasMember("y") && defaults["y"].IsNumber()) t->position.y() = defaults["y"].GetFloat();
//        if (defaults.HasMember("w") && defaults["w"].IsNumber()) t->scale.x() = defaults["w"].GetFloat();
//        if (defaults.HasMember("h") && defaults["h"].IsNumber()) t->scale.y() = defaults["h"].GetFloat();
//        if (defaults.HasMember("angle") && defaults["angle"].IsNumber()) t->rotation = defaults["angle"].GetFloat();
//        if (defaults.HasMember("height") && defaults["height"].IsNumber()) t->height = defaults["height"].GetFloat();
//    }
//    // Example: Override transform position if specified in defaults
//    if (defaults.HasMember("position") && defaults["position"].IsArray()) {
//        Transform* transform_ptr = entity_component_based_system.getComponent<Transform>(entity);
//        if (transform_ptr != nullptr) {
//            const auto& position_array = defaults["position"].GetArray();
//            if (position_array.Size() >= 2) {
//                //transform_ptr->position.x = position_array[0].GetFloat();
//                //transform_ptr->position.y = position_array[1].GetFloat();
//            }
//        }
//    }
//}

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




