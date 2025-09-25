#include "./scene/GameObject.h"
#include <map>

//is this ok??????????????????????????????????
//std::map<std::string, std::unique_ptr<GameObject>> PREFAB_LIST;

//prefab parses game objects from a json folder
//which is then accessed by the Factory when making game scenes
class PrefabManager {
private:

	std::map<std::string, std::unique_ptr<GameObject>> list;
public:
	PrefabManager(std::string filename = "assets/prefabs/GameObjs.json");

	GameObject* operator[](const char* key)const;

};