#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <concepts>

#include "Resource.h"
#include "ECS/ECS_Component.h"
#include "Audio.h"
#include "Font.h"

class AudioSystem;
class ECS;

class ResourceManager {
public:
	enum class ResourceType {
		AUDIO,
		// Add other resource types here

		NONE
	};

	ResourceManager(ECS& ecs, AudioSystem* audioSystem = nullptr);
	~ResourceManager();

	ResourceManager(ResourceManager const& other) = delete;
	ResourceManager& operator=(ResourceManager const& other) = delete;
	ResourceManager(ResourceManager&& other) = delete;
	ResourceManager& operator=(ResourceManager&& other) = delete;

	void loadResources(std::string const& resourceFilePath = "");

	Audio& loadAudio(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	Audio& getAudio(ResourceID id);

	void insertUsedID(ResourceID& id);

	std::unordered_map<ResourceID, Audio>& getAudios();

	template <typename T>
	ResourceID findResource(const std::string& ResourceName);

	template <typename T>
	void getResourceMetaData(ResourceType resourceType, ResourceID id, T callback);

	template <typename T>
	std::pair<std::string, std::string> getName(ResourceID id);

	ResourceID getNewAssetID();
	Audio defaultAudio;

	void setAudioSystem(AudioSystem* audioSystem);

	const Audio& getDefaultAudio() const;

private:
	ECS& ecs;
	std::unordered_map<ResourceID, Audio> audios;

	AudioSystem* audioSystem{ nullptr };

	ResourceID totalResourceID = 1;
	std::unordered_set<ResourceID> usedIDs;
};

template <typename T>
ResourceID ResourceManager::findResource(const std::string& ResourceName) {
	if constexpr (std::same_as<T, Audio>) {
		auto it = std::find_if(audios.begin(), audios.end(), [&](std::pair<const ResourceID, Audio> const& pair) {
			return pair.second.getKey() == ResourceName;
		});

		if (it == audios.end()) return NO_RESOURCE_ID;

		return it->first;
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Unsupported resource type in findResource");
		}();
	}
}

template <typename T>
void ResourceManager::getResourceMetaData(ResourceType resourceType, ResourceID id, T callback) {
	switch (resourceType) {
		using enum ResourceType;
	case AUDIO:
		callback(getAudio(id), getAudios());
		break;

	default:
		break;
	}
}

template <typename T>
std::pair<std::string, std::string> ResourceManager::getName(ResourceID id) {
	if constexpr (std::same_as<T, Audio>) {
		auto it = audios.find(id);
		if (it == audios.end()) {
			return { "Audio", "Default" };
		}
		return { "Audio",audios.at(id).getKey() };
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Template argument is not a Resource!");
		}();
	}
}


#endif // RESOURCEMANAGER_H