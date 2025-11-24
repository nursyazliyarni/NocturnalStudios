/*!------------------------------------------------------------------------------------------------
	File: ResourceManager.h
	Project: NocturnalStudio

	This file contains the implementation of the ResourceManager class

	Primary Author:
		Lau Xiang Yang (95%)
		- Created the ResourceManager class to manage all resources in the engine

	Secondary Author:
		Nur Syazliyarni (5%)
		- Added a function for the asset browser to be able to add new 
		textures from the file path

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

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

/**
 * @brief Central manager for game resources (audio, shaders, textures, fonts, prefabs, scenes).
 *
 * Responsibilities:
 *  - Discover and load resource metadata from disk.
 *  - Provide lookup access to resource records by ResourceID or key.
 *  - Maintain unique ResourceID allocation and a registry of used IDs.
 *  - Optionally forward audio loads to an AudioSystem so FMOD assets are prepared.
 *
 * Notes:
 *  - ResourceManager stores only metadata and host-side resource descriptors (e.g. file paths).
 *    Uploading GPU/audio runtime objects is handled by the respective subsystems (Renderer, AudioSystem).
 *  - Most operations are not thread-safe; external synchronization is required for concurrent mutation.
 */
class ResourceManager {
public:
    /**
     * @brief Type enum for selecting resource containers in generic helpers.
     */
    enum class ResourceType {
        AUDIO,
        SHADER,
        TEXTURE,
        FONT,
        PREFAB,
        SCENE,
        NONE
    };

    /**
     * @brief Construct a ResourceManager.
     * @param ecs Reference to the engine's ECS instance (used by some resource operations).
     * @param audioSystem Optional pointer to an AudioSystem. If provided, newly created Audio records
     *                    will be forwarded to the AudioSystem to create FMOD::Sound objects.
     */
    ResourceManager(ECS& ecs, AudioSystem* audioSystem = nullptr);

    /**
     * @brief Destructor. Intended to perform any final serialization/cleanup.
     */
    ~ResourceManager();

    ResourceManager(ResourceManager const& other) = delete;
    ResourceManager& operator=(ResourceManager const& other) = delete;
    ResourceManager(ResourceManager&& other) = delete;
    ResourceManager& operator=(ResourceManager&& other) = delete;

    /**
     * @brief Scan the assets directory (or a named subfolder) and load supported resources.
     * @param resourceFilePath Optional subfolder under "assets" (e.g. "audio", "textures"). Empty => scan "assets".
     *
     * Behavior:
     *  - Walks assets/<resourceFilePath> recursively and invokes the appropriate loadX() for each supported file.
     *  - Adds metadata records to internal maps; does not necessarily create runtime GPU/audio objects (AudioSystem is used for FMOD).
     */
    void loadResources(std::string const& resourceFilePath = "");

    /**
     * @brief Recursively scan and (re)load audio files from a directory.
     * @param directoryPath Filesystem path to scan.
     * @return Number of new audio resources added.
     *
     * The function validates file extensions and will skip unsupported formats.
     */
    std::size_t reloadAudiosFromDirectory(const std::string& directoryPath);
    /**
     * @brief Recursively scan and (re)load texture files from a directory.
     * @param directoryPath Filesystem path to scan.
     * @return Number of new texture resources added.
     */
    std::size_t reloadTexturesFromDirectory(const std::string& directoryPath);

    /**
     * @brief Create or return an existing Audio resource record.
     * @param name Human-readable key for the resource.
     * @param filePath Filesystem path to the audio file.
     * @param id Optional ResourceID; if NEW_RESOURCE a new id is allocated.
     * @return Reference to the stored Audio record (may be the defaultAudio on error).
     *
     * Notes:
     *  - If an AudioSystem is attached it will be informed (AudioSystem::loadSound) for immediate FMOD preparation.
     */
    Audio& loadAudio(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
    /**
     * @brief Retrieve an Audio resource by ResourceID.
     * @param id ResourceID to lookup.
     * @return Reference to the Audio or defaultAudio if not found.
     */
    Audio& getAudio(ResourceID id);

    ShaderFile& loadShader(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
    ShaderFile& getShader(ResourceID id);

    TextureFile& loadTexture(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
    TextureFile& getTexture(ResourceID id);

    /**
     * @brief Load a font resource and prepare glyphs for the requested pixel size.
     * @param name Resource key.
     * @param filePath Filesystem path to font file (.ttf/.otf).
     * @param pixelSize Pixel height for glyphs to generate.
     * @param id Optional ResourceID.
     * @return Reference to the stored Font record.
     */
    Font& loadFont(const std::string& name, const std::string& filePath, unsigned int pixelSize, ResourceID id = NEW_RESOURCE);
    Font& getFont(ResourceID id);

    PrefabFile& loadPrefab(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE);
    PrefabFile& getPrefab(ResourceID id);

    SceneFile& loadScene(const std::string& name, const std::string& filePath, ResourceID id = NEW_RESOURCE, bool startWithThisScene = false);
    SceneFile& getScene(ResourceID id);

    /**
     * @brief Mark a ResourceID as already used.
     * @param id ResourceID to reserve.
     *
     * Use when restoring serialized metadata to avoid reusing IDs.
     */
    void insertUsedID(ResourceID& id);

    // Accessors for the internal maps (mutable references)
    std::unordered_map<ResourceID, Audio>& getAudios();
    std::unordered_map<ResourceID, ShaderFile>& getShaders();
    std::unordered_map<ResourceID, TextureFile>& getTextures();
    std::unordered_map<ResourceID, Font>& getFonts();
    std::unordered_map<ResourceID, PrefabFile>& getPrefabs();
    std::unordered_map<ResourceID, SceneFile>& getScenes();
    const std::unordered_map<ResourceID, SceneFile>& getScenes() const;

    /**
     * @brief Cached GL textures loaded for the ImGui/asset browser.
     * Keyed by absolute file path to the image.
     */
    std::unordered_map<std::string, std::unique_ptr<Texture>> loadedGLTextures;

    /**
     * @brief Return file paths for all loaded resources of `type` whose key equals `key`.
     * @param type ResourceType enum selecting container.
     * @param key Human-readable resource key to match.
     * @return Vector of file paths (may be empty).
     */
    std::vector<std::string> getFilePathsForKey(ResourceType type, const std::string& key) const;

    /**
     * @brief Templated convenience: return file paths for resources of concrete type T that match `key`.
     * @tparam T Concrete resource type (Audio, ShaderFile, TextureFile, Font, PrefabFile, SceneFile).
     * @param key Human-readable resource key.
     * @return Vector of matching file paths.
     */
    template <typename T>
    std::vector<std::string> getFilePathsForKey(const std::string& key) const;

    /**
     * @brief Find a resource by its human-readable key and return its ResourceID.
     * @tparam T Concrete resource type to search.
     * @param ResourceName Key to match.
     * @return ResourceID for the first match, or NO_RESOURCE_ID if not found.
     *
     * Note: performs a linear search.
     */
    template <typename T>
    ResourceID findResource(const std::string& ResourceName);

    /**
     * @brief Invoke a callback with the resource instance and its container map.
     * @tparam T Callable type.
     * @param resourceType Which container to query.
     * @param id ResourceID to lookup.
     * @param callback Callable invoked as callback(resource, container).
     *
     * Useful for generic UI or inspector code that needs both the resource and the full map.
     */
    template <typename T>
    void getResourceMetaData(ResourceType resourceType, ResourceID id, T callback);

    /**
     * @brief Return {typeName, key} pair for a resource id.
     * @tparam T Concrete resource type.
     * @param id ResourceID to query.
     * @return Pair where first is a human-friendly type string and second is the resource key.
     */
    template <typename T>
    std::pair<std::string, std::string> getName(ResourceID id);

    /**
     * @brief Allocate a new, unused ResourceID.
     * @return New ResourceID guaranteed not to be in usedIDs at the time of return.
     */
    ResourceID getNewAssetID();

    /**
     * @brief The built-in default Audio returned on lookup failures.
     */
    Audio defaultAudio;

    /**
     * @brief Attach or replace the AudioSystem used to forward audio loads.
     * @param audioSystem Pointer to AudioSystem (may be nullptr).
     */
    void setAudioSystem(AudioSystem* audioSystem);

    /**
     * @brief Const accessor for the default audio record.
     */
    const Audio& getDefaultAudio() const;

    /**
     * @brief Load an image into an OpenGL texture and cache it for the asset browser.
     * @param filePath Filesystem path to image.
     * @return GL texture ID (0 on failure).
     */
    GLuint getGLTextureFromFile(const std::string& filePath);
    /**
     * @brief Clear the cached GL textures.
     */
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

// Templated functions --------------------------------------------------------

/**
 * @brief Find a resource by its human-readable name/key.
 *
 * Template specialization currently supports:
 *  - Audio, ShaderFile, TextureFile, Font, PrefabFile, SceneFile
 *
 * @tparam T The concrete resource type to search for (e.g. Audio).
 * @param ResourceName The resource key to find.
 * @return ResourceID for the first match or NO_RESOURCE_ID if not found.
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
 * The callback signature should accept (Resource&, std::unordered_map<ResourceID, Resource>&).
 * Example:
 *   rm.getResourceMetaData([](Audio &a, auto &map){ /* UI code here *\/ });
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
 * Supported T: Audio, ShaderFile, TextureFile, Font, PrefabFile, SceneFile.
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
 * Supported T: Audio, ShaderFile, TextureFile, Font, PrefabFile, SceneFile.
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