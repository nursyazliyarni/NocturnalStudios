#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <concepts>

#include "../include/resources/Audio.h"

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

	void loadResources(std::string const& resourceFilePath = "");
	
	Audio& loadAudio(const std::string& name, const std::string& filePath, );
	Audio& getAudio();
};


#endif // RESOURCEMANAGER_H