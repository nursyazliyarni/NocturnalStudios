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


#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "./scene/Prefab.h"
//#include "./scene/Component.h"
#include "ECS/ECS.h"

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

EntityID PrefabManager::Instantiate(const std::string& prefab_name, const std::string& entity_name) {
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

    // Transform component
    if (component_type == "Transform") {
        Transform transform_component;
        if (component_json.HasMember("position") && component_json["position"].IsArray()) {
            const auto& position_array = component_json["position"].GetArray();
            if (position_array.Size() >= 2) {
                //transform_component.position.x = position_array[0].GetFloat();
                //transform_component.position.y = position_array[1].GetFloat();
            }
        }
        if (component_json.HasMember("scale") && component_json["scale"].IsArray()) {
            const auto& scale_array = component_json["scale"].GetArray();
            if (scale_array.Size() >= 2) {
                //transform_component.scale.x = scale_array[0].GetFloat();
                //transform_component.scale.y = scale_array[1].GetFloat();
            }
        }
        if (component_json.HasMember("rotation") && component_json["rotation"].IsNumber()) {
            transform_component.rotation = component_json["rotation"].GetFloat();
        }
        if (component_json.HasMember("height") && component_json["height"].IsNumber()) {
            transform_component.height = component_json["height"].GetFloat();
        }
        entity_component_based_system.addComponent<Transform>(entity, transform_component);
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
    else if (component_type == "PhysicsComponent") {
        PhysicsComponent physics_component;
        if (component_json.HasMember("velocity") && component_json["velocity"].IsArray()) {
            const auto& velocity_array = component_json["velocity"].GetArray();
            if (velocity_array.Size() >= 2) {
                //physics_component.velocity.x = velocity_array[0].GetFloat();
                //physics_component.velocity.y = velocity_array[1].GetFloat();
            }
        }
        if (component_json.HasMember("gravityEnabled") && component_json["gravityEnabled"].IsBool()) {
            physics_component.gravityEnabled = component_json["gravityEnabled"].GetBool();
        }
        entity_component_based_system.addComponent<PhysicsComponent>(entity, physics_component);
    }
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

void PrefabManager::ApplyComponentDefaults(EntityID entity, const rapidjson::Value& defaults) {
    // This function can be used to apply runtime overrides to components
    // For example, setting specific positions or properties when instantiating

    // Example: Override transform position if specified in defaults
    if (defaults.HasMember("position") && defaults["position"].IsArray()) {
        Transform* transform_ptr = entity_component_based_system.getComponent<Transform>(entity);
        if (transform_ptr != nullptr) {
            const auto& position_array = defaults["position"].GetArray();
            if (position_array.Size() >= 2) {
                //transform_ptr->position.x = position_array[0].GetFloat();
                //transform_ptr->position.y = position_array[1].GetFloat();
            }
        }
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



