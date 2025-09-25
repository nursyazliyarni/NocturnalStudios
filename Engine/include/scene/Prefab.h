#include "./scene/GameObject.h"

//prefab parses game objects from a json folder
//which is then accessed by the Factory when making game scenes
namespace PrefabManager {
	bool LoadObjects(std::string filename = "assets/prefabs/GameObjs.json");
}