
#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "./scene/Prefab.h"
#include "./scene/Component.h"

//prefab wil load the gameobjects from a file, 
//and have a copy of one of each of the object in here?

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
				//std::cout << "Name: " << item["name"].GetString() << std::endl;
				std::string name{ item["name"].GetString() };
				list.emplace(name, std::make_unique<GameObject>(name));
				GameObject* obj = list[name].get();
				if (item["components"].IsArray())
				{
					for (const auto& comp : item["components"].GetArray()) {

						//need to add error checking 
						if (comp["type"].IsString())
						{ 
							std::unique_ptr<Component> component{};
							component = ComponentFactory::TryCreate(comp["type"].GetString(), comp);
							if (component)
							{
								obj->Attach(std::move(component));
							}
						}


					}
				}
				//std::cout << "GameObject Registered: " << name << '\n';
			}
		}
	}
		
		
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


