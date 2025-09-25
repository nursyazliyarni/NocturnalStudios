
#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "./scene/Prefab.h"

//prefab wil load the gameobjects from a file, 
//and have a copy of one of each of the object in here?

PrefabManager::PrefabManager(std::string filename) :list{} {
	std::ifstream objFile(filename);
	if (objFile)
	{
		rapidjson::IStreamWrapper obj(objFile);
		rapidjson::Document doc;
		doc.ParseStream(obj);

		if (doc.IsArray()) {
			for (const auto& item : doc.GetArray()) {
				std::cout << "Name: " << item["name"].GetString() << std::endl;
				std::string name{ item["name"].GetString() };
				list.emplace(name, std::make_unique<GameObject>(name));
				if (item["components"].IsArray())
				{
					for (const auto& comp : item["components"].GetArray()) {

						


					}
				}

			}
		}
	}
		
		
}


