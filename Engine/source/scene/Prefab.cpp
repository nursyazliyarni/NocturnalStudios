
#include "rapidjson/document.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include "./scene/Prefab.h"

//prefab wil load the gameobjects from a file, 
//and have a copy of one of each of the object in here?
namespace PrefabManager {
	bool LoadObjects(std::string filename) {
		std::ifstream objFile(filename);
		if (!objFile) return false;

		rapidjson::IStreamWrapper obj(objFile);
		rapidjson::Document doc;
		doc.ParseStream(obj);

		if (doc.IsArray()) {
			for (const auto& item : doc.GetArray()) {
				std::cout << "Name: " << item["name"].GetString() << std::endl;
			}
		}
		return true;
	}
}

