
// Add filepath here
constexpr char DEFAULT_AUDIO_FILEPATH[] = "DefaultAudio";

#include <iostream>
#include <filesystem>
#include <../fmod/fmod_errors.h>

#include "ECS/ECS.h"
#include "resources/ResourceManager.h"
#include "core/Logger.h"

extern Logger logger;

ResourceManager::ResourceManager(ECS& ecs) : defaultAudio{ 0, "Default", DEFAULT_AUDIO_FILEPATH }, ecs{ ecs } {
	// Deserialize the asset meta data here (TODO)
}

ResourceManager::~ResourceManager() {
	// Serialize the asset meta data here (TODO)
}

void ResourceManager::loadResources(std::string const& resourceFilePath) {
	std::cout << "Loading resources\n";

	std::string resourcePath{ "Resources" };
	std::string directoryPath{ resourceFilePath };

	if (!directoryPath.empty()) {
		resourcePath += "/" + directoryPath;
	}

	for (auto const& entry : std::filesystem::recursive_directory_iterator(resourcePath)) {
		auto const& path = entry.path();
		auto const& stem = path.stem();
		auto const& filename = path.filename();

		if (!filename.has_extension()) continue;

		// Check Resource type here
		if (resourceFilePath == "Audio") {
			loadAudio(stem.string(), path.string());
		}
	}

	std::cout << "Finished loading resources\n" << std::endl;
}

Audio& ResourceManager::loadAudio(const std::string& name, const std::string& filePath, ResourceID id) {
	for (auto& [key, audio] : audios) {
		if (std::filesystem::path{ audio.getFilePath() } == std::filesystem::path{ filePath }) {
			return audio;
		}
	}

	if (id == NEW_RESOURCE) id = getNewAssetID();
	audios.emplace(id, Audio{ id, name, filePath });

	logger.log("Loaded Audio: " + name + " from " + filePath);
	return audios.at(id);
}

Audio& ResourceManager::getAudio(ResourceID id) {
	auto found = audios.find(id);
	if (found == audios.end()) {
		logger.error("No key found, default audio returned");
		return defaultAudio;
	}
	return found->second;
}

void ResourceManager::insertUsedID(ResourceID& id) {
	auto [it, success] = usedIDs.insert(id);

	if (!success) {
		logger.warning("Trying to insert a used ResourceID: " + std::to_string(id));
	}
}

std::unordered_map<ResourceID, Audio>& ResourceManager::getAudios() {
	return audios;
}

ResourceID ResourceManager::getNewAssetID() {
	while (usedIDs.find(totalResourceID) != usedIDs.end()) {
		++totalResourceID;
	}

	usedIDs.insert(totalResourceID);
	return totalResourceID;
}
