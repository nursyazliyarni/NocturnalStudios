#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <concepts>

#include "Resource.h"
#include "Audio.h"
#include "Font.h"
#include "ShaderFile.h"

class AudioSystem;
class ECS;

class ResourceManager {
public:
	enum class ResourceType {
		// Add other resource types here
		AUDIO,
		SHADER,
		TEXTURE,
		FONT,

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

	ShaderFile& loadShader(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	ShaderFile& getShader(ResourceID id);

	void insertUsedID(ResourceID& id);

	std::unordered_map<ResourceID, Audio>& getAudios();
	std::unordered_map<ResourceID, ShaderFile>& getShaders();

	template <typename T>
	ResourceID findResource(const std::string& ResourceName);

	template <typename T>
	void getResourceMetaData(ResourceType resourceType, ResourceID id, T callback);

	template <typename T>
	std::pair<std::string, std::string> getName(ResourceID id);

	ResourceID getNewAssetID();
	Audio defaultAudio;

	void setAudioSystem(AudioSystem* audioSystem);

private:
	ECS& ecs;

	std::unordered_map<ResourceID, Audio> audios;
	std::unordered_map<ResourceID, ShaderFile> shaderFiles;

	AudioSystem* audioSystem{ nullptr };

	ResourceID totalResourceID = 1;
	std::unordered_set<ResourceID> usedIDs;
};

/**
 * @brief Find a resource by its human-readable name/key.
 *
 * Template specialization currently supports:
 *  - `Audio`
 *  - `ShaderFile`
 *
 * @tparam T The concrete resource type to search for (e.g. `Audio`).
 * @param ResourceName The resource key/name to find.
 * @return The `ResourceID` for the first matching resource, or `NO_RESOURCE_ID`
 *         if no resource with the given name is found.
 *
 * Notes:
 *  - This performs a linear search over the container for the given resource
 *    type (O(n)). Use and store the returned `ResourceID` for repeated access.
 *  - Passing an unsupported type triggers a compile-time static_assert with a
 *    helpful message.
 */
template <typename T>
ResourceID ResourceManager::findResource(const std::string& ResourceName) {
	if constexpr (std::same_as<T, Audio>) {
		auto it = std::find_if(audios.begin(), audios.end(), [&](std::pair<const ResourceID, Audio> const& pair) {
			return pair.second.getKey() == ResourceName;
		});

		if (it == audios.end()) return NO_RESOURCE_ID;

		return it->first;
	}
	else if constexpr (std::same_as<T, ShaderFile>) {
		auto it = std::find_if(shaderFiles.begin(), shaderFiles.end(), [&](std::pair<const ResourceID, ShaderFile> const& pair) {
			return pair.second.getKey() == ResourceName;
			});

		if (it == shaderFiles.end()) return NO_RESOURCE_ID;

		return it->first;
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Unsupported resource type in findResource");
		}();
	}
}

/**
 * @brief Invoke a callback with metadata for a resource type and id.
 *
 * This function dispatches based on the `ResourceType` enum and forwards
 * the requested resource instance and the internal container for that type
 * to the provided callback.
 *
 * Example callback signature for AUDIO:
 *   [](Audio &audio, std::unordered_map<ResourceID, Audio> &audios) { ... }
 *
 * @tparam T Callable type of the callback. The callback should accept the
 *           concrete resource reference as the first parameter and a reference
 *           to the internal container of that resource type as the second
 *           parameter (see example above).
 * @param resourceType The `ResourceType` enum value indicating which internal
 *                     container to use.
 * @param id The `ResourceID` of the resource to query.
 * @param callback A callable invoked with (resource, container).
 *
 * Notes:
 *  - If the resource id is invalid the underlying `getAudio/getX` should
 *    handle fallback behavior (for `Audio` this may return `defaultAudio`).
 */
template <typename T>
void ResourceManager::getResourceMetaData(ResourceType resourceType, ResourceID id, T callback) {
	switch (resourceType) {
		using enum ResourceType;
	case AUDIO:
		callback(getAudio(id), getAudios());
		break;
	case SHADER:
		callback(getShader(id), getShaders());
	default:
		break;
	}
}

/**
 * @brief Get a human-friendly type/name pair for a resource id.
 *
 * Template specialization currently supports:
 *  - `Audio` -> returns pair { "Audio", key }
 *
 * @tparam T The concrete resource type to query (e.g. `Audio`).
 * @param id ResourceID to lookup.
 * @return pair where:
 *    - first: human readable resource type name (e.g. "Audio")
 *    - second: the resource key/name for that id (or a sensible default if not found)
 *
 * Notes:
 *  - Calling with an unsupported template type triggers a compile-time
 *    static_assert with a helpful message.
 */
template <typename T>
std::pair<std::string, std::string> ResourceManager::getName(ResourceID id) {
	if constexpr (std::same_as<T, Audio>) {
		auto it = audios.find(id);
		if (it == audios.end()) {
			return { "Audio", "Default" };
		}
		return { "Audio",audios.at(id).getKey() };
	}
	else if constexpr (std::same_as<T, ShaderFile>) {
		auto it = shaderFiles.find(id);
		if (it == shaderFiles.end()) {
			return { "ShaderFile", "Default" };
		}
		return { "ShaderFile",shaderFiles.at(id).getKey() };
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Template argument is not a Resource!");
		}();
	}
}


#endif // RESOURCEMANAGER_H