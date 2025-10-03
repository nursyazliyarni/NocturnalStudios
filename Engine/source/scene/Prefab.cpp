/* !------------------------------------------------------------------------------------------------
	File: Prefab.cpp
	Project : NocturnalStudio

	This file contains the class definitions for a prefab manager and loader

	Primary Author :
		Laycock Enhui Sarah (e.laycock@digipen.edu) (100%)
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

void RegisterAllComponents() {
	ComponentFactory::RegisterType<CircleRenderer>("CircleRenderer");
	ComponentFactory::RegisterType<RectRenderer>("RectRenderer");
	ComponentFactory::RegisterType<SpriteRenderer>("SpriteRenderer");
	ComponentFactory::RegisterType<AABBCollider>("AABBCollider");
	ComponentFactory::RegisterType<AnimalBehaviour>("AnimalBehaviour");
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
							component = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
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
	if (it!=list.end())
	{
		return it->second.get();
	}
	else {
		return nullptr;
	}
}



