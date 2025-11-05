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

// [0]=ESC, [1..9]=digits
static std::array<SceneID, 10> sKey2Scene{};
static bool sKeymapInit = false;

// sceneName -> SceneID (built from JSON "scenes" key order aligned to in-memory scenes vector)
static std::unordered_map<std::string, SceneID> sNameToId;

// configurable JSON path (fallback stays this)
static std::string sScenesJsonPath = "assets/scenes/scenes.json";

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
    std::cout << "ADDING SOMETHING\n";

    std::string component_type = component_json["type"].GetString();

    if (component_type == "SpriteRenderer") component_type = "ImageRenderer";
    if (component_type == "AABBCollider")  component_type = "BoxCollider";


    // Transform component
    //if (component_type == "Transform") {
    //    Transform transform_component;
    //    if (component_json.HasMember("position") && component_json["position"].IsArray()) {
    //        const auto& position_array = component_json["position"].GetArray();
    //        if (position_array.Size() >= 2) {
    //            //transform_component.position.x = position_array[0].GetFloat();
    //            //transform_component.position.y = position_array[1].GetFloat();
    //        }
    //    }
    //    if (component_json.HasMember("scale") && component_json["scale"].IsArray()) {
    //        const auto& scale_array = component_json["scale"].GetArray();
    //        if (scale_array.Size() >= 2) {
    //            //transform_component.scale.x = scale_array[0].GetFloat();
    //            //transform_component.scale.y = scale_array[1].GetFloat();
    //        }
    //    }
    //    if (component_json.HasMember("rotation") && component_json["rotation"].IsNumber()) {
    //        transform_component.rotation = component_json["rotation"].GetFloat();
    //    }
    //    if (component_json.HasMember("height") && component_json["height"].IsNumber()) {
    //        transform_component.height = component_json["height"].GetFloat();
    //    }
    //    entity_component_based_system.addComponent<Transform>(entity, transform_component);
    //}
    if (component_type == "Transform") {
        Transform t{};
        if (component_json.HasMember("position") && component_json["position"].IsArray()) {
            const auto& a = component_json["position"].GetArray();
            if (a.Size() >= 2) { t.position.x() = a[0].GetFloat(); t.position.y() = a[1].GetFloat(); }
        }
        if (component_json.HasMember("scale") && component_json["scale"].IsArray()) {
            const auto& a = component_json["scale"].GetArray();
            if (a.Size() >= 2) { t.scale.x() = a[0].GetFloat(); t.scale.y() = a[1].GetFloat(); }
        }
        if (component_json.HasMember("rotation") && component_json["rotation"].IsNumber()) {
            t.rotation = component_json["rotation"].GetFloat();
        }
        if (component_json.HasMember("height") && component_json["height"].IsNumber()) {
            t.height = component_json["height"].GetFloat();
        }
        entity_component_based_system.addComponent<Transform>(entity, t);
    }
    else if (component_type == "PhysicsComponent") {
        PhysicsComponent pc{};
        if (component_json.HasMember("velocity") && component_json["velocity"].IsArray()) {
            const auto& v = component_json["velocity"].GetArray();
            if (v.Size() >= 2) { pc.velocity.x() = v[0].GetFloat(); pc.velocity.y() = v[1].GetFloat(); }
        }
        if (component_json.HasMember("gravityEnabled") && component_json["gravityEnabled"].IsBool()) {
            pc.gravityEnabled = component_json["gravityEnabled"].GetBool();
        }
        entity_component_based_system.addComponent<PhysicsComponent>(entity, pc);
    }


    // ImageRenderer component
    else if (component_type == "ImageRenderer") {
        ImageRenderer renderer_component;
        if (component_json.HasMember("textureID") && component_json["textureID"].IsInt()) {
            renderer_component.textureID = component_json["textureID"].GetInt();
        }
        if (component_json.HasMember("opacity") && component_json["opacity"].IsNumber()) {
            renderer_component.opacity = component_json["opacity"].GetFloat();
        }
        if (component_json.HasMember("isVisible") && component_json["isVisible"].IsBool()) {
            renderer_component.isVisible = component_json["isVisible"].GetBool();
        }

        if (renderer_component.textureID <= 0) renderer_component.isVisible = false; // << add this

        entity_component_based_system.addComponent<ImageRenderer>(entity, renderer_component);
    }

    // CircleRenderer component
    else if (component_type == "CircleRenderer") {
        CircleRenderer renderer_component;
        if (component_json.HasMember("radius") && component_json["radius"].IsNumber()) {
            renderer_component.radius = component_json["radius"].GetFloat();
        }
        if (component_json.HasMember("colorTint") && component_json["colorTint"].IsArray()) {
            const auto& color_array = component_json["colorTint"].GetArray();
            if (color_array.Size() >= 4) {
                /*renderer_component.colorTint.x = color_array[0].GetFloat();
                renderer_component.colorTint.y = color_array[1].GetFloat();
                renderer_component.colorTint.z = color_array[2].GetFloat();
                renderer_component.colorTint.w = color_array[3].GetFloat();*/
            }
        }
        entity_component_based_system.addComponent<CircleRenderer>(entity, renderer_component);
    }
    // RectRenderer component
    else if (component_type == "RectRenderer") {
        RectRenderer renderer_component;
        if (component_json.HasMember("width") && component_json["width"].IsNumber()) {
            renderer_component.width = component_json["width"].GetFloat();
        }
        if (component_json.HasMember("height") && component_json["height"].IsNumber()) {
            renderer_component.height = component_json["height"].GetFloat();
        }
        entity_component_based_system.addComponent<RectRenderer>(entity, renderer_component);
    }
    // PhysicsComponent
    //else if (component_type == "PhysicsComponent") {
    //    PhysicsComponent physics_component;
    //    if (component_json.HasMember("velocity") && component_json["velocity"].IsArray()) {
    //        const auto& velocity_array = component_json["velocity"].GetArray();
    //        if (velocity_array.Size() >= 2) {
    //            //physics_component.velocity.x = velocity_array[0].GetFloat();
    //            //physics_component.velocity.y = velocity_array[1].GetFloat();
    //        }
    //    }
    //    if (component_json.HasMember("gravityEnabled") && component_json["gravityEnabled"].IsBool()) {
    //        physics_component.gravityEnabled = component_json["gravityEnabled"].GetBool();
    //    }
    //    entity_component_based_system.addComponent<PhysicsComponent>(entity, physics_component);
    //}
    
    // BoxCollider
    else if (component_type == "BoxCollider") {
        BoxCollider collider_component;
        if (component_json.HasMember("width") && component_json["width"].IsNumber()) {
            collider_component.width = component_json["width"].GetFloat();
        }
        if (component_json.HasMember("height") && component_json["height"].IsNumber()) {
            collider_component.height = component_json["height"].GetFloat();
        }
        entity_component_based_system.addComponent<BoxCollider>(entity, collider_component);
    }
    // AnimalBehavior
    else if (component_type == "AnimalBehavior") {
        AnimalBehavior behavior_component;
        if (component_json.HasMember("sound") && component_json["sound"].IsString()) {
            behavior_component.sound = component_json["sound"].GetString();
        }
        entity_component_based_system.addComponent<AnimalBehavior>(entity, behavior_component);
    }
    // Add more component types as needed...
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


//static void RegisterAllComponentsOnce() {
//    static bool s_done = false;
//    if (s_done) return;
//    ComponentFactory::RegisterType<CircleRenderer>("CircleRenderer");
//    ComponentFactory::RegisterType<RectRenderer>("RectRenderer");
//    ComponentFactory::RegisterType<SpriteRenderer>("SpriteRenderer");
//    ComponentFactory::RegisterType<AABBCollider>("AABBCollider");
//    ComponentFactory::RegisterType<AnimalBehavior>("AnimalBehavior");
//    s_done = true;
//}

//PrefabManager::PrefabManager(ECS& ecs) : entity_component_based_system(ecs) {}
//
//
////prefab wil load the gameobjects from a file, 
////and have a copy of one of the object in here?
//
//void PrefabManager::Load_Prefab(const std::string& filename_path) {
//    std::ifstream objFile(filename_path);
//    if (!objFile) {
//        return;
//    }
//
//    rapidjson::IStreamWrapper obj(objFile);
//    rapidjson::Document doc;
//    doc.ParseStream(obj);
//
//    if (doc.HasParseError() || !doc.IsObject()) return;
//
//    prefab_map[filename_path] = std::move(doc);
//    std::cout << filename_path << "\n";
//}
//
////PrefabManager::PrefabManager(std::string filename) :list{} {
////    {
////        // ensure factory has types
////        //RegisterAllComponents();
////        RegisterAllComponentsOnce();
////
////        std::ifstream objFile(filename);
////        if (!objFile) {
////            return;
////        }
////
////        rapidjson::IStreamWrapper obj(objFile);
////        rapidjson::Document doc;
////        doc.ParseStream(obj);
////
////        // ----- NEW SCHEMA: { "prefabs": { "<name>": { "components":[...], "defaults":{...} }, ... } }
////        if (doc.HasMember("prefabs") && doc["prefabs"].IsObject()) {
////            const auto& prefabs = doc["prefabs"];
////            for (auto it = prefabs.MemberBegin(); it != prefabs.MemberEnd(); ++it) {
////                const std::string name = it->name.GetString();
////                const rapidjson::Value& def = it->value;
////
////                // create the prefab prototype GameObject
////                list.emplace(name, std::make_unique<GameObject>(name));
////                GameObject* objGO = list[name].get();
////
////                // components
////                if (def.HasMember("components") && def["components"].IsArray()) {
////                    for (const auto& comp : def["components"].GetArray()) {
////                        if (comp.HasMember("type") && comp["type"].IsString()) {
////                            std::unique_ptr<Component> c = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
////                            if (c) {
////                                objGO->Attach(std::move(c));
////                            }
////                        }
////                    }
////                }
////
////                // NOTE: def["defaults"] exists in your JSON; we keep it for future
////                // (transform system to apply w/h/angle later) — intentionally not wired now.
////            }
////            return; // done with new schema
////        }
////        if (doc.IsArray()) {
////            for (const auto& item : doc.GetArray()) {
////                if (!item.HasMember("name") || !item["name"].IsString()) continue;
////                const std::string name{ item["name"].GetString() };
////
////                list.emplace(name, std::make_unique<GameObject>(name));
////                GameObject* objGO = list[name].get();
////
////                if (item.HasMember("components") && item["components"].IsArray()) {
////                    for (const auto& comp : item["components"].GetArray()) {
////                        if (comp.HasMember("type") && comp["type"].IsString()) {
////                            std::unique_ptr<Component> c = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
////                            if (c) {
////                                objGO->Attach(std::move(c));
////                            }
////                        }
////                    }
////                }
////            }
////        }
////    }	
////}
//
//PrefabManager::~PrefabManager() {
//    list.clear(); // unique_ptr will delete all GameObjects
//}
//
//const GameObject* PrefabManager::operator[](const char* key)const {
//	auto it = list.find(key);
//	if (it!=list.end())
//	{
//		//return it->second.get();
//	}
//	else {
//		return nullptr;
//	}
//}



