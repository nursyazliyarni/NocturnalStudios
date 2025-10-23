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

#include <cstdio>     // for file* fopen
#include <rapidjson/filereadstream.h>


void RegisterAllComponents() {
	ComponentFatory::RegisterType<CircRenderer>("CircleRenderer");
	ComponentFatory::RegisterType<RectaRenderer>("RectRenderer");
	ComponentFatory::RegisterType<SpriteRenderer>("SpriteRenderer");
	ComponentFatory::RegisterType<AABBCollider>("AABBCollider");
	ComponentFatory::RegisterType<AnimalBehaviou>("AnimalBehaviour");
}

//prefab wil load the gameobjects from a file, 
//and have a copy of one of the object in here?

PrefabManager::PrefabManager(std::string filename) :list{} {
	//std::cout << "Prefab manager init " << filename << '\n';
	std::ifstream objFile(filename);

	if (objFile)
	{
		//std::cout << "Prefab manager opened " << filename << '\n';
		rapidjson::IStreamWrapper obj(objFile);
		rapidjson::Document doc;
		doc.ParseStream(obj);

		if (doc.IsArray()) {
			for (const auto& item : doc.GetArray()) {
				//std::cout << "Creating Gameobj. Name: " << item["name"].GetString();
				std::string name{ item["name"].GetString() };
				list.emplace(name, std::make_unique<GameObject>(name));
				GameObject* obj = list[name].get();
				if (item["components"].IsArray())
				{
					//std::cout << "|| Components: ";
					for (const auto& comp : item["components"].GetArray()) {

						//need to add error checking 
						if (comp["type"].IsString())
						{
							//std::cout << comp["type"].GetString()<<"{";
							std::unique_ptr<Component> component{};
							component = ComponentFatory::TryCreate(comp["type"].GetString(), comp);
							if (component)
							{
								obj->Attach(std::move(component));
								//std::cout << "Component Attatched: " << comp["type"].GetString() << std::endl;
							}
							//std::cout <<"} ";
						}


					}
					//std::cout << "||";
				}
				//std::cout <<  '\n';
			}
		}
	}


}

PrefabManager::~PrefabManager() {
	list.clear(); // unique_ptr will delete all GameObjects
}

const GameObject* PrefabManager::operator[](const char* key)const {
	auto it = list.find(key);
	if (it != list.end())
	{
		return it->second.get();
	}
	else {
		return nullptr;
	}
}



bool PrefabManager::loadSpriteArchetypes(
	const std::string& filename,
	const std::unordered_map<std::string, size_t>& texTagToIndex)
{
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp) {
		logger.error(std::string{ "Prefab JSON not found: " } + filename);
		return false;
	}

	char buffer[4096];
	rapidjson::FileReadStream is(fp, buffer, sizeof(buffer));
	rapidjson::Document d;
	d.ParseStream(is);
	fclose(fp);

	if (d.HasParseError() || !d.HasMember("sprites") || !d["sprites"].IsObject()) {
		logger.error(std::string{ "Invalid prefab JSON format in " } + filename);
		return false;
	}

	const auto& spritesObj = d["sprites"];
	for (auto it = spritesObj.MemberBegin(); it != spritesObj.MemberEnd(); ++it) {
		const std::string prefabName = it->name.GetString();    // e.g. "Food"
		const auto& node = it->value;                            // object

		if (!node.IsObject() || !node.HasMember("tag") || !node["tag"].IsString()) {
			logger.warning(std::string{ "Prefab '" } + prefabName + "' missing 'tag' — skipped");
			continue;
		}

		const std::string tag = node["tag"].GetString();         // e.g. "food"

		auto texIt = texTagToIndex.find(tag);
		if (texIt == texTagToIndex.end()) {
			logger.warning(std::string{ "Prefab '" } + prefabName + "' tag '" + tag + "' has no texture — skipped");
			continue;
		}

		SpriteArchetype arch;
		arch.texId = static_cast<int>(texIt->second);

		// Optional w/h/draggable (default to 0/0/true if not present)
		if (node.HasMember("w") && node["w"].IsNumber()) arch.w = node["w"].GetFloat();
		if (node.HasMember("h") && node["h"].IsNumber()) arch.h = node["h"].GetFloat();
		if (node.HasMember("draggable") && node["draggable"].IsBool()) arch.draggable = node["draggable"].GetBool();
		if (node.HasMember("rotatable") && node["rotatable"].IsBool()) arch.rotatable = node["rotatable"].GetBool();
		if (node.HasMember("scalable") && node["scalable"].IsBool()) arch.scalable = node["scalable"].GetBool();


		sprites[prefabName] = arch;

		// log once
		{
			std::stringstream ss;
			ss << "Prefab '" << prefabName << "' -> tag '" << tag << "' (texId=" << arch.texId
				<< "), w=" << arch.w << ", h=" << arch.h
				<< ", draggable=" << (arch.draggable ? "true" : "false")
				<< ", rotatable=" << (arch.rotatable ? "true" : "false")
				<< ", scalable=" << (arch.scalable ? "true" : "false");

			logger.log(ss.str());
		}

		logger.log("Prefab archetype data stored successfully for " + prefabName);

	}

	return true;
}
