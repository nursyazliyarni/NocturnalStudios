/* !------------------------------------------------------------------------------------------------
	File: Prefab.cpp
	Project : NocturnalStudio

	This file contains ...

	 Author :
		Sarah(%)
		- Brief

	 Author :
		Name(%) Rishika Bimal Attodi
		- stabilized init & main loop, wired systems, 
        fixed menu menu to scene transitions, and cleaned up redundant code paths.

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
PrefabManager::PrefabManager(ECS& ecs) : entity_component_based_system(ecs), prefab_map{} {
    
}

void PrefabManager::Load_Prefab(const std::string& filename_path) {
    std::ifstream objFile(filename_path);
    if (!objFile) {
        logger.error("Failed to open prefab file: " + filename_path);
        return;
    }

    rapidjson::IStreamWrapper obj(objFile);
    rapidjson::Document document;
    document.ParseStream(obj);

    if (document.HasParseError() || !document.IsArray()) {
        logger.error("Failed to parse prefab file: " + filename_path);
        return;
    }
     //prefab file contains an array of prefabs which have 
    //name and an array of components
    unsigned int unnamed_prefab_count{};
    for (rapidjson::Value& obj:document.GetArray())
    {
        std::string prefab_name{};
        if (obj.HasMember("name")&&obj["name"].IsString())
            prefab_name = obj["name"].GetString();
        else
        {
            prefab_name = "Prefab_" + std::to_string(unnamed_prefab_count);
            unnamed_prefab_count++;
        }

        EntityID prefabID{ entity_component_based_system.createEntity(prefab_name) };
        if (prefabID == INVALID_ENTITY_ID) continue;

        //next, attach components
        if (obj.HasMember("components")&&obj["components"].IsArray())
        {
            for (rapidjson::Value& comp : obj["components"].GetArray()) 
            {
                AddComponentFromJSON(prefabID, comp);
            }
        }

        //when done, add the entity to the prefab map
        prefab_map.emplace(prefab_name, prefabID);
    }


}

/**
 * Given the name of a prefab (e.g processing in a scene file), creates an entity with that name.
 * 
 * \param prefab_name
 * \param object_name
 * \return 
 */
EntityID PrefabManager::CreateEntityFromPrefab(const std::string& prefab_name,
    const std::string& object_name)
{
    //1. Verify prefab file loaded
    if (prefab_map.empty()) {
        logger.error("Prefab file empty");
        return INVALID_ENTITY_ID;
    }
    logger.log("Prefab map populated");


    auto prefabIt = prefab_map.find(prefab_name);
    if (prefabIt==prefab_map.end())
    {
        logger.error("Could not find Prefab: "+prefab_name);
        return INVALID_ENTITY_ID;
    }
    logger.log("Prefab found: " + prefab_name);


    //prefab with name exists
    EntityID entity=entity_component_based_system.cloneEntity(prefabIt->second);

    logger.log("Naming new Object");
    if (!object_name.empty()&&entity!=INVALID_ENTITY_ID)
    {
        entity_component_based_system.getEntityName(entity) = object_name;
    }
    
    //if it returns a invalid id at this point, fault is on ECS
    return entity;
}


bool PrefabManager::PrefabExists(const std::string& prefab_name) const {
    auto prefabIt = prefab_map.find(prefab_name);
    if (prefabIt == prefab_map.end())
    {
        return false;
    }
    else return true;
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






