#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <concepts>

#include "resources/Re"

class ECS;

class ResourceManager {
public:
	enum class ResourceType {
		AUDIO,
		// Add other resource types here

		NONE
	};

	ResourceManager(ECS& ecs);
	~ResourceManager();

	ResourceManager(ResourceManager const& other) = delete;
	ResourceManager& operator=(ResourceManager const& other) = delete;
	ResourceManager(ResourceManager&& other) = delete;
	ResourceManager& operator=(ResourceManager&& other) = delete;

	void loadResources(std::string const& resourceFilePath = "");

	Audio& loadAudio(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	Audio& getAudio(ResourceID id);
};


#endif // RESOURCEMANAGER_H