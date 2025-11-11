#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <concepts>

#define UNREFERENCED_PARAMETER(x) (void)(x)

#include "Resource.h"
#include "Audio.h"
#include "Font.h"
#include "ShaderFile.h"
#include "TextureFile.h"
#include "PrefabFile.h"
#include "SceneFile.h"
#include "graphics/Texture.h"

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
		PREFAB,
		SCENE,

		NONE
	};

	ResourceManager(ECS& ecs, AudioSystem* audioSystem = nullptr);
	~ResourceManager();

	ResourceManager(ResourceManager const& other) = delete;
	ResourceManager& operator=(ResourceManager const& other) = delete;
	ResourceManager(ResourceManager&& other) = delete;
	ResourceManager& operator=(ResourceManager&& other) = delete;

	void loadResources(std::string const& resourceFilePath = "");

	std::size_t reloadAudiosFromDirectory(const std::string& directoryPath);
	std::size_t reloadTexturesFromDirectory(const std::string& directoryPath);

	Audio& loadAudio(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	Audio& getAudio(ResourceID id);

	ShaderFile& loadShader(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	ShaderFile& getShader(ResourceID id);

	TextureFile& loadTexture(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	TextureFile& getTexture(ResourceID id);

	Font& loadFont(const std::string& name, const std::string& filePath, unsigned int pixelSize, ResourceID id = NEW_RESOURCE);
	Font& getFont(ResourceID id);

	PrefabFile& loadPrefab(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
	PrefabFile& getPrefab(ResourceID id);

	SceneFile& loadScene(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE, bool startWithThisScene = false);
	SceneFile& getScene(ResourceID id);

	void insertUsedID(ResourceID& id);

	std::unordered_map<ResourceID, Audio>& getAudios();
	std::unordered_map<ResourceID, ShaderFile>& getShaders();
	std::unordered_map<ResourceID, TextureFile>& getTextures();
	std::unordered_map<ResourceID, Font>& getFonts();
	std::unordered_map<ResourceID, PrefabFile>& getPrefabs();
	std::unordered_map<ResourceID, SceneFile>& getScenes();
	const std::unordered_map<ResourceID, SceneFile>& getScenes() const;
	std::unordered_map<std::string, std::unique_ptr<Texture>> loadedGLTextures;

	std::vector<std::string> getFilePathsForKey(ResourceType type, const std::string& key) const;

	template <typename T>
	std::vector<std::string> getFilePathsForKey(const std::string& key) const;

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

	GLuint getGLTextureFromFile(const std::string& filePath);
	void clearAllGLTexturesCache();
	
private:
	ECS& ecs;

	std::unordered_map<ResourceID, Audio> audios;
	std::unordered_map<ResourceID, ShaderFile> shaderFiles;
	std::unordered_map<ResourceID, TextureFile> textureFiles;
	std::unordered_map<ResourceID, Font> fonts;
	std::unordered_map<ResourceID, PrefabFile> prefabFiles;
	std::unordered_map<ResourceID, SceneFile> sceneFiles;

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
 *  - `TextureFile`
 *  - `Font`
 *  - `PrefabFile`
 *  - `SceneFile`
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
	else if constexpr (std::same_as<T, TextureFile>) {
		auto it = std::find_if(textureFiles.begin(), textureFiles.end(), [&](std::pair<const ResourceID, TextureFile> const& pair) {
			return pair.second.getKey() == ResourceName;
			});
		if (it == textureFiles.end()) return NO_RESOURCE_ID;
		return it->first;
	}
	else if constexpr (std::same_as<T, Font>) {
		auto it = std::find_if(fonts.begin(), fonts.end(), [&](std::pair<const ResourceID, Font> const& pair) {
			return pair.second.getKey() == ResourceName;
			});
		if (it == fonts.end()) return NO_RESOURCE_ID;
		return it->first;
	}
	else if constexpr (std::same_as<T, PrefabFile>) {
		auto it = std::find_if(prefabFiles.begin(), prefabFiles.end(), [&](std::pair<const ResourceID, PrefabFile> const& pair) {
			return pair.second.getKey() == ResourceName;
			});
		if (it == prefabFiles.end()) return NO_RESOURCE_ID;
		return it->first;
	}
	else if constexpr (std::same_as<T, SceneFile>) {
		auto it = std::find_if(sceneFiles.begin(), sceneFiles.end(), [&](std::pair<const ResourceID, SceneFile> const& pair) {
			return pair.second.getKey() == ResourceName;
			});
		if (it == sceneFiles.end()) return NO_RESOURCE_ID;
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
		break;
	case TEXTURE:
		callback(getTexture(id), getTextures());
		break;
	case FONT:
		callback(getFont(id), getFonts());
		break;
	case PREFAB:
		callback(getPrefab(id), getPrefabs());
		break;
	case SCENE:
		callback(getScene(id), getScenes());
		break;
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
	else if constexpr (std::same_as<T, TextureFile>) {
		auto it = textureFiles.find(id);
		if (it == textureFiles.end()) {
			return { "TextureFile", "Default" };
		}
		return { "TextureFile",textureFiles.at(id).getKey() };
	}
	else if constexpr (std::same_as<T, Font>) {
		auto it = fonts.find(id);
		if (it == fonts.end()) {
			return { "Font", "Default" };
		}
		return { "Font",fonts.at(id).getKey() };
	}
	else if constexpr (std::same_as<T, PrefabFile>) {
		auto it = prefabFiles.find(id);
		if (it == prefabFiles.end()) {
			return { "PrefabFile", "Default" };
		}
		return { "PrefabFile",prefabFiles.at(id).getKey() };
	}
	else if constexpr (std::same_as<T, SceneFile>) {
		auto it = sceneFiles.find(id);
		if (it == sceneFiles.end()) {
			return { "SceneFile", "Default" };
		}
		return { "SceneFile",sceneFiles.at(id).getKey() };
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Template argument is not a Resource!");
		}();
	}
}

/**
 * @brief Return file paths for all loaded resources that match a given key.
 *
 * This templated helper walks the internal container for the concrete resource
 * type `T` and collects `filePath` values for every resource whose `key`
 * equals the supplied `key` parameter.
 *
 * Supported template types:
 *  - `Audio`
 *  - `ShaderFile`
 *  - `TextureFile`
 *  - `Font`
 *  - `PrefabFile`
 *  - `SceneFile`
 *
 * @tparam T Concrete resource type (see supported types above).
 * @param key Human-readable resource key to match (compared with `Resource::getKey()`).
 * @return std::vector<std::string> List of file paths (`Resource::getFilePath()`)
 *         for all matching resources. Returns an empty vector if none match.
 *
 * Complexity: O(n) over the number of loaded resources of type `T`.
 *
 * Notes:
 *  - If an unsupported `T` is provided, a compile-time `static_assert` triggers
 *    with the message "Template argument is not a Resource!".
 *  - This does an equality comparison on the key; if you need case-insensitive
 *    or fuzzy matching perform that externally before calling this function.
 */
template <typename T>
std::vector<std::string> ResourceManager::getFilePathsForKey(const std::string& key) const {
	std::vector<std::string> filePaths;

	if constexpr (std::same_as<T, Audio>) {
		for (auto const& [id, file] : audios) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else if constexpr (std::same_as<T, ShaderFile>) {
		for (auto const& [id, file] : shaderFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else if constexpr (std::same_as<T, TextureFile>) {
		for (auto const& [id, file] : textureFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else if constexpr (std::same_as<T, Font>) {
		for (auto const& [id, file] : fonts) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else if constexpr (std::same_as<T, PrefabFile>) {
		for (auto const& [id, file] : prefabFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else if constexpr (std::same_as<T, SceneFile>) {
		for (auto const& [id, file] : sceneFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
	}
	else {
		[]<bool flag = false>() {
			static_assert(flag, "Template argument is not a Resource!");
		}();
	}

	return filePaths;
}

#endif // RESOURCEMANAGER_H