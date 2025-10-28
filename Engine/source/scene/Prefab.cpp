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
#include "./scene/Component.h"

//void RegisterAllComponents() {
//	ComponentFactory::RegisterType<CircleRenderer>("CircleRenderer");
//	ComponentFactory::RegisterType<RectRenderer>("RectRenderer");
//	ComponentFactory::RegisterType<SpriteRenderer>("SpriteRenderer");
//	ComponentFactory::RegisterType<AABBCollider>("AABBCollider");
//	ComponentFactory::RegisterType<AnimalBehaviour>("AnimalBehaviour");
//}

static void RegisterAllComponentsOnce() {
    static bool s_done = false;
    if (s_done) return;
    ComponentFactory::RegisterType<CircleRenderer>("CircleRenderer");
    ComponentFactory::RegisterType<RectRenderer>("RectRenderer");
    ComponentFactory::RegisterType<SpriteRenderer>("SpriteRenderer");
    ComponentFactory::RegisterType<AABBCollider>("AABBCollider");
    ComponentFactory::RegisterType<AnimalBehavior>("AnimalBehavior");
    s_done = true;
}


//prefab wil load the gameobjects from a file, 
//and have a copy of one of the object in here?

PrefabManager::PrefabManager(std::string filename) :list{} {
    {
        // ensure factory has types
        //RegisterAllComponents();
        RegisterAllComponentsOnce();

        std::ifstream objFile(filename);
        if (!objFile) {
            return;
        }

        rapidjson::IStreamWrapper obj(objFile);
        rapidjson::Document doc;
        doc.ParseStream(obj);

        // ----- NEW SCHEMA: { "prefabs": { "<name>": { "components":[...], "defaults":{...} }, ... } }
        if (doc.HasMember("prefabs") && doc["prefabs"].IsObject()) {
            const auto& prefabs = doc["prefabs"];
            for (auto it = prefabs.MemberBegin(); it != prefabs.MemberEnd(); ++it) {
                const std::string name = it->name.GetString();
                const rapidjson::Value& def = it->value;

                // create the prefab prototype GameObject
                list.emplace(name, std::make_unique<GameObject>(name));
                GameObject* objGO = list[name].get();

                // components
                if (def.HasMember("components") && def["components"].IsArray()) {
                    for (const auto& comp : def["components"].GetArray()) {
                        if (comp.HasMember("type") && comp["type"].IsString()) {
                            std::unique_ptr<Component> c = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
                            if (c) {
                                objGO->Attach(std::move(c));
                            }
                        }
                    }
                }

                // NOTE: def["defaults"] exists in your JSON; we keep it for future
                // (transform system to apply w/h/angle later) — intentionally not wired now.
            }
            return; // done with new schema
        }
        if (doc.IsArray()) {
            for (const auto& item : doc.GetArray()) {
                if (!item.HasMember("name") || !item["name"].IsString()) continue;
                const std::string name{ item["name"].GetString() };

                list.emplace(name, std::make_unique<GameObject>(name));
                GameObject* objGO = list[name].get();

                if (item.HasMember("components") && item["components"].IsArray()) {
                    for (const auto& comp : item["components"].GetArray()) {
                        if (comp.HasMember("type") && comp["type"].IsString()) {
                            std::unique_ptr<Component> c = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
                            if (c) {
                                objGO->Attach(std::move(c));
                            }
                        }
                    }
                }
            }
        }
    }	
}

PrefabManager::~PrefabManager() {
    list.clear(); // unique_ptr will delete all GameObjects
}

const GameObject* PrefabManager::operator[](const char* key)const {
	auto it = list.find(key);
	if (it!=list.end())
	{
		return it->second.get();
	}
	else {
		return nullptr;
	}
}



