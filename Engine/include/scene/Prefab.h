//prefab.h
#include "./scene/GameObject.h"
#include <map>
#include <unordered_map> 

//is this ok??????????????????????????????????
//std::map<std::string, std::unique_ptr<GameObject>> PREFAB_LIST;

//prefab parses game objects from a json folder
//which is then accessed by the Factory when making game scenes
//shld be made static

struct SpriteArchetype {
	int texId{ 0 };
	//float w{ 0.12f }, h{ 0.18f }; // default normalized size
	float w{ 0.f }, h{ 0.f };   // set by JSON; 0.f means “not specified”
	bool  draggable{ true };  // from JSON (items true, animals false)
	bool rotatable = false;	
	bool scalable = false;
	bool movable = false;
};


class PrefabManager {
private:

	std::map<std::string, std::unique_ptr<GameObject>> list;
	std::unordered_map<std::string, SpriteArchetype> sprites;

public:
	PrefabManager(std::string filename = "assets/prefabs/GameObj.json");
	~PrefabManager(); // Added destructor

	const GameObject* operator[](const char* key)const;


	void registerSprite(const std::string& name, const SpriteArchetype& a) {
		sprites[name] = a;
	}
	const SpriteArchetype* getSprite(const std::string& name) const {
		auto it = sprites.find(name);
		return (it == sprites.end()) ? nullptr : &it->second;
	}

	// Load sprite archetypes (Food/Drink/Toy/Flamingo/...) from JSON.
// Resolves "tag" to texId using the provided tag to  index map.
	bool loadSpriteArchetypes(
		const std::string& filename,
		const std::unordered_map<std::string, size_t>& texTagToIndex);

	// Iterate all sprite archetypes (name + data)
// (header-only helper used by Application.cpp to build a draggable set)
	template<typename F>
	void forEachSprite(F&& fn) const {
		for (const auto& kv : sprites) fn(kv.first, kv.second);
	}

	// Return reference to all sprite archetypes (used for prefab-driven rotation)
	const std::unordered_map<std::string, SpriteArchetype>& getAllSprites() const {
		return sprites;
	}


};

void RegisterAllComponents();