#include "./scene/GameObject.h"
#include <map>

//is this ok??????????????????????????????????
//std::map<std::string, std::unique_ptr<GameObject>> PREFAB_LIST;

//prefab parses game objects from a json folder
//which is then accessed by the Factory when making game scenes
//shld be made static
class PrefabManager {
private:

	std::map<std::string, std::unique_ptr<GameObject>> list;
public:
	PrefabManager(std::string filename = "assets/prefabs/GameObj.json");

	const GameObject* operator[](const char* key)const;

};

void RegisterAllComponents();