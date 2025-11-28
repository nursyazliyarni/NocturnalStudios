/*!------------------------------------------------------------------------------------------------
	File: ResourceManager.cpp
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

// Add filepath here
// constexpr char DEFAULT_AUDIO_FILEPATH[] = "DefaultAudio/filepath";

#define _HAS_STD_BYTE 0

#include <iostream>
#include <filesystem>
#include <algorithm>
#include "fmod_errors.h"

#include "ECS/ECS.h"
#include "resources/ResourceManager.h"
#include "core/Logger.h"
#include "System/Audio_System.h"
#include "graphics/Texture.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <windows.h>
#endif

extern Logger logger;

namespace {
    static const std::array<const char*, 6> kSupportedAudioExt = { ".wav", ".ogg", ".mp3", ".flac", ".aiff", ".fsb" };

    static bool IsSupportedAudioExtension(const std::string& extLower)
    {
        for (auto* e : kSupportedAudioExt)
            if (extLower == e) return true;
        return false;
    }

    static void ShowAudioErrorPopup(const std::string& msg)
    {
    #ifdef _WIN32
        GLFWwindow* gw = glfwGetCurrentContext();
        HWND parent = gw ? glfwGetWin32Window(gw) : nullptr;
        MessageBoxA(parent, msg.c_str(), "Audio Format Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
    #else
        // Fallback: could integrate an ImGui modal if needed for other platforms.
        logger.error(msg);
    #endif
    }
}

/**
 * ResourceManager
 *
 * Constructs a ResourceManager which owns a reference to the project's ECS and
 * an optional AudioSystem pointer.
 *
 * Parameters:
 *  - ecs: Reference to the engine ECS. The ResourceManager uses this when
 *         resource-related entities/components need to be created or queried.
 *  - audioSystem: Optional pointer to an AudioSystem. If provided, the
 *         ResourceManager forwards audio load requests to the AudioSystem so
 *         FMOD resources are prepared.
 *
 * Side-effects:
 *  - Initializes the builtin/default audio entry `defaultAudio`.
 *  - (TODO) Intended to deserialize resource metadata (placeholder).
 *
 * Simple usage:
 *  ECS ecs;
 *  AudioSystem* audioSys = /* create or get AudioSystem * /;
 *  ResourceManager rm(ecs, audioSys);
 */
ResourceManager::ResourceManager(ECS& ecs, AudioSystem* audioSystem) 
	: ecs{ ecs }, audioSystem{ audioSystem } {
}

/**
 * ~ResourceManager
 *
 * Destructor. Intended to perform cleanup and serialize resource metadata.
 *
 * Side-effects:
 *  - (TODO) Serialize asset metadata to disk if required.
 *
 * Simple usage:
 *  // Automatic when ResourceManager goes out of scope or during engine shutdown.
 */
ResourceManager::~ResourceManager() {

}

/**
 * setAudioSystem
 *
 * Assigns or replaces the AudioSystem instance used by the ResourceManager.
 *
 * Parameters:
 *  - audioSystemPtr: pointer to the AudioSystem. May be nullptr to remove the
 *    association.
 *
 * Side-effects:
 *  - Future audio loads will be forwarded to the provided AudioSystem if not null.
 *
 * Simple usage:
 *  ResourceManager rm(ecs);
 *  rm.setAudioSystem(audioSys); // now audio loads inform the AudioSystem
 */
void ResourceManager::setAudioSystem(AudioSystem* audioSystemPtr) {
	audioSystem = audioSystemPtr;
}

/**
 * loadResources
 *
 * Searches the Resources directory (and subdirectories) for files of the
 * type indicated by `resourceFilePath` and loads them. Currently supports
 * "Audio" as a resourceFilePath value.
 *
 * Parameters:
 *  - resourceFilePath: subdirectory under "assets" to scan. If empty,
 *    defaults to scanning "assets".
 *
 * Behavior:
 *  - Walks the directory tree under "assets[/resourceFilePath]". 
 *  - For each file that has an extension and matches a supported resource
 *    type, calls the appropriate loader (e.g., loadAudio).
 *
 * Simple usage:
 *  rm.loadResources();           // scan Resources and load supported resource types
 *  rm.loadResources("Audio");    // only scan assets/Audio and load audio files
 */
void ResourceManager::loadResources(std::string const& resourceFilePath) {
	logger.log("Beginning resource loading for: " + resourceFilePath + " ...");

	std::string resourcePath{ "assets" };
	std::string directoryPath{ resourceFilePath };

	if (!directoryPath.empty()) {
		resourcePath += "/" + directoryPath;
	}

	// Case-insensitive type compare (keep original path casing for disk paths)
	std::string typeKey = directoryPath;
	std::transform(typeKey.begin(), typeKey.end(), typeKey.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });


	for (auto const& entry : std::filesystem::recursive_directory_iterator(resourcePath)) {
		auto const& path = entry.path();
		auto const& stem = path.stem();
		auto const& filename = path.filename();

		if (!filename.has_extension()) continue;

		// Check Resource type here
		//if (resourceFilePath == "audio") {
		//	loadAudio(stem.string(), path.string());
		//}
		//else if (resourceFilePath == "shaders") {
		//	loadShader(stem.string(), path.string());
		//}
		//else if (resourceFilePath == "textures") {
		//	loadTexture(stem.string(), path.string());
		//}
		//else if (resourceFilePath == "fonts") {
		//	std::string ext = path.extension().string();
		//	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) 
		//		{ return static_cast<char>(std::tolower(c)); });
		//	if (ext == ".ttf" || ext == ".otf") {
		//		loadFont(stem.string(), path.string(), 48); // default 48px; adjust per project needs
		//	}
		//}
		//else if (resourceFilePath == "prefabs") {
		//	loadPrefab(stem.string(), path.string());
		//}
		//else if (resourceFilePath == "scenes") {
		//	loadScene(stem.string(), path.string());
		//}

		if (typeKey == "audio") {
			loadAudio(stem.string(), path.string());
		}
		else if (typeKey == "shaders") {
			loadShader(stem.string(), path.string());
		}
		else if (typeKey == "textures") {
			loadTexture(stem.string(), path.string());
		}
		else if (typeKey == "fonts") {
			std::string ext = path.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
				{ return static_cast<char>(std::tolower(c)); });
			if (ext == ".ttf" || ext == ".otf") {
				loadFont(stem.string(), path.string(), 48);
			}
		}
		else if (typeKey == "prefabs") {
			loadPrefab(stem.string(), path.string());
		}
		else if (typeKey == "scenes") {
			loadScene(stem.string(), path.string());
		}

	}

	logger.log("Resource loading complete.");
}

/**
 * getFilePathsForKey
 *
 * Retrieves the file paths for all loaded resources of type `T` that match
 * the provided key.
 *
 * Supported resource types `T`:
 *  - See ResourceManager.h for the full list:
 *   Audio, ShaderFile, TextureFile, Font, PrefabFile, SceneFile
 * 
 * Parameters:
 *  - key: the resource key to search for.
 * 
 * Returns:
 *  - std::vector<std::string> containing the file paths of all matching
 *   resources. If no resources match, returns an empty vector.
 * 
 * Behavior:
 * - Iterates over the internal storage for resources of type `T`.
 * - Compares each resource's key with the provided `key`.
 * - Collects and returns the file paths of all matching resources.
 */
std::size_t ResourceManager::reloadAudiosFromDirectory(const std::string& directoryPath) {
	namespace fs = std::filesystem;
	std::size_t added = 0;
	static const char* extensions[] = { ".wav", ".ogg", ".mp3", ".flac", ".aiff", ".fsb" };

	if (!fs::exists(directoryPath)) {
		logger.error("Audio directory not found: " + directoryPath);
		return 0;
	}

	for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
		if (!entry.is_regular_file()) continue;

		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (!IsSupportedAudioExtension(ext)) {
			ShowAudioErrorPopup("Unsupported audio file format:\n" + entry.path().string() + "\nAllowed: .wav .ogg .mp3 .flac .aiff .fsb");
			logger.error("Blocked loading unsupported audio format: " + entry.path().string());
			continue;
		}

		const std::string key = entry.path().stem().string();
		const std::string path = entry.path().string();

		if (this->findResource<Audio>(key) != NO_RESOURCE_ID) continue;

		Audio& audio = this->loadAudio(key, path);
		UNREFERENCED_PARAMETER(audio);
		++added;
	}

	return added;
}

/**
 * reloadTexturesFromDirectory
 * 
 * Scans the specified directory for texture files and loads any new textures
 * that are not already present in the ResourceManager.
 * 
 * Parameters:
 * - directoryPath: The filesystem path to the directory to scan for texture files.
 * 
 * Returns:
 * - The number of new textures that were loaded.
 * 
 * Behavior:
 * - Recursively iterates through the specified directory.
 * - For each file, checks if its extension matches a supported texture format.
 * - If a texture with the same key (filename without extension) is not already
 * present, it loads the texture using loadTexture.
 * - Supported texture formats include: .png, .jpg, .jpeg, .bmp, .tga, .gif, .hdr, .psd.
 * - Logs an error if the specified directory does not exist.
 */
std::size_t ResourceManager::reloadTexturesFromDirectory(const std::string& directoryPath) {
    namespace fs = std::filesystem;
    std::size_t added = 0;
    static const char* extensions[] = { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".gif", ".hdr", ".psd" };

    if (!fs::exists(directoryPath)) {
        logger.error("Textures directory not found: " + directoryPath);
        return 0;
    }

    for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
        if (!entry.is_regular_file()) continue;

        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        bool supported = std::any_of(std::begin(extensions), std::end(extensions),
            [&](const char* e) { return ext == e; });
        if (!supported) continue;

        const std::string key = entry.path().stem().string();
        const std::string path = entry.path().string();

        if (this->findResource<TextureFile>(key) != NO_RESOURCE_ID) continue;

        TextureFile& tex = this->loadTexture(key, path);
        UNREFERENCED_PARAMETER(tex);
        ++added;
    }

    return added;
}

/**
 * loadAudio
 *
 * Load or reuse an Audio resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory Audio record exists per file path. If an
 *    Audio with an identical file path is already present, that Audio is
 *    returned. Otherwise a new Audio object is constructed and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the audio (e.g. "BGM_Main").
 *  - filePath: Filesystem path to the audio file.
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *
 * Returns:
 *  - Reference to the stored Audio instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `audios` to detect an existing audio by
 *    comparing std::filesystem::path values. This comparison is syntactic
 *    (string-based); it does not resolve symlinks or canonicalize paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts a new `Audio` into the `audios` map with the chosen ID.
 *  - Logs the load operation through the global `logger`.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `audios` or ID state
 *    require external synchronization.
 *
 *
 * Notes:
 *  - This function only manages the Audio resource metadata stored in the
 *    ResourceManager. It does not call into `AudioSystem` or initialize any
 *    FMOD objects. If FMOD sound objects are required, call the appropriate
 *    AudioSystem API (e.g., `AudioSystem::loadSound`) after obtaining the
 *    returned Audio reference.
 *
 * Example:
 *  Audio& a = rm.loadAudio("MainTheme", "Resources/Audio/MainTheme.mp3");
 *  // If you need the FMOD sound:
 *  // if (rm.getAudioSystem()) rm.getAudioSystem()->loadSound(a);
 */
Audio& ResourceManager::loadAudio(const std::string& name, const std::string& filePath, ResourceID id) {
    // Extension validation before creating the resource
    std::filesystem::path fp{ filePath };
    std::string ext = fp.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (!IsSupportedAudioExtension(ext)) {
        ShowAudioErrorPopup("Unsupported audio file format:\n" + filePath + "\nAllowed: .wav .ogg .mp3 .flac .aiff .fsb");
        logger.error("Blocked loading unsupported audio format: " + filePath);
    }

	for (auto& [key, audio] : audios) {
		if (std::filesystem::path{ audio.getFilePath() } == std::filesystem::path{ filePath }) {
			return audio;
		}
	}

	if (id == NEW_RESOURCE) id = getNewAssetID();
	audios.emplace(id, Audio{ id, name, filePath });

	logger.log("Loaded Audio: " + name + " from " + filePath);

	// If an AudioSystem is attached, forward the newly created Audio so FMOD
	// can create the FMOD::Sound immediately. This makes it easy to call
	// loadAudio(...) after setAudioSystem(...) from application code.
	if (audioSystem) {
		try {
			audioSystem->loadSound(audios.at(id));
		} catch (...) {
			// avoid throwing from resource manager; log and continue
			logger.error("Failed to forward audio to AudioSystem for: " + name);
		}
	}

	return audios.at(id);
}

/**
 * getAudio
 *
 * Retrieves a previously loaded Audio by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found Audio object.
 *  - If no matching Audio exists, returns `defaultAudio` (and logs an error).
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  Audio& a = rm.getAudio(id); // returns defaultAudio if id not found
 */
Audio& ResourceManager::getAudio(ResourceID id) {
	auto found = audios.find(id);
	if (found == audios.end()) {
		logger.error("No key found, default audio returned");
	}
	return found->second;
}

/**
 * loadShader
 *
 * Load or reuse a ShaderFile resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory ShaderFile record exists per file path. If a
 *    ShaderFile with an identical file path is already present, that
 *    ShaderFile is returned. Otherwise a new ShaderFile object is constructed
 *    and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the shader (e.g. "StandardLit").
 *  - filePath: Filesystem path to the shader file.
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *
 * Returns:
 *  - Reference to the stored ShaderFile instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `shaderFiles` to detect an existing shader by
 *    comparing std::filesystem::path values. This comparison is syntactic
 *    (string-based); it does not resolve symlinks or canonicalize paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts a new `ShaderFile` into the `shaderFiles` map with the chosen ID.
 *  - Logs the load operation through the global `logger`.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `shaderFiles` or ID state
 *    require external synchronization.
 *
 * Example:
 *  ShaderFile& s = rm.loadShader("Standard", "assets/shaders/standard.glsl");
 */
ShaderFile& ResourceManager::loadShader(const std::string& name, const std::string& filePath, ResourceID id) {
	for (auto& [key, shaderFile] : shaderFiles) {
		if (std::filesystem::path{ shaderFile.getFilePath() } == std::filesystem::path{ filePath }) {
			return shaderFile;
		}
	}

	if (id == NEW_RESOURCE) id = getNewAssetID();

	shaderFiles.emplace(id, ShaderFile{ id, name, filePath });
	logger.log("Loaded Shader: " + name + " from " + filePath);
	return shaderFiles.at(id);
}

/**
 * getShader
 *
 * Retrieve a previously loaded ShaderFile by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found ShaderFile object.
 *  - If no matching ShaderFile exists the function asserts (caller should
 *    ensure id validity) � consistent with existing usage in this codebase.
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  ShaderFile& s = rm.getShader(id);
 */
ShaderFile& ResourceManager::getShader(ResourceID id) {
	auto found = shaderFiles.find(id);
	assert(found != shaderFiles.end());
	return found->second;
}

/**
 * loadTexture
 *
 * Load or reuse a TextureFile resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory TextureFile record exists per file path. If a
 *    TextureFile with an identical file path is already present, that
 *    TextureFile is returned. Otherwise a new TextureFile object is constructed
 *    and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the texture (e.g. "BrickAlbedo").
 *  - filePath: Filesystem path to the texture file.
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *
 * Returns:
 *  - Reference to the stored TextureFile instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `textureFiles` to detect an existing texture by
 *    comparing std::filesystem::path values. This comparison is syntactic
 *    (string-based); it does not resolve symlinks or canonicalize paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts a new `TextureFile` into the `textureFiles` map with the chosen ID.
 *  - Logs the load operation through the global `logger`.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `textureFiles` or ID state
 *    require external synchronization.
 *
 * Example:
 *  TextureFile& t = rm.loadTexture("Brick", "assets/textures/brick.png");
 */
TextureFile& ResourceManager::loadTexture(const std::string& name, const std::string& filePath, ResourceID id) {
	for (auto& [key, textureFile] : textureFiles) {
		if (std::filesystem::path{ textureFile.getFilePath() } == std::filesystem::path{ filePath }) {
			return textureFile;
		}
	}
	if (id == NEW_RESOURCE) id = getNewAssetID();

	textureFiles.emplace(id, TextureFile{ id, name, filePath });
	logger.log("Loaded Texture: " + name + " from " + filePath);
	return textureFiles.at(id);
}

/**
 * getTexture
 *
 * Retrieve a previously loaded TextureFile by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found TextureFile object.
 *  - If no matching TextureFile exists the function asserts (caller should
 *    ensure id validity) � consistent with existing usage in this codebase.
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  TextureFile& t = rm.getTexture(id);
 */
TextureFile& ResourceManager::getTexture(ResourceID id) {
	auto found = textureFiles.find(id);
	assert(found != textureFiles.end());
	return found->second;
}

/**
 * loadFont
 *
 * Load or reuse a Font resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory Font record exists per file path + pixel size.
 *    If a Font with an identical file path and pixelSize is already present,
 *    that Font is returned. Otherwise a new Font object is constructed and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the font (e.g. "Roboto-Regular").
 *  - filePath: Filesystem path to the font file.
 *  - pixelSize: The pixel height to load glyphs for (font size used when
 *               generating glyphs / GPU resources).
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *
 * Returns:
 *  - Reference to the stored Font instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `fonts` to detect an existing font by
 *    comparing std::filesystem::path values and pixelSize. This comparison is
 *    syntactic (string-based); it does not resolve symlinks or canonicalize
 *    paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts a new `Font` into the `fonts` map using `try_emplace` and asserts
 *    that insertion succeeds.
 *  - Calls `Font::LoadFont(...)` to upload font glyphs / GPU resources for the
 *    requested pixelSize. If glyph loading fails, an error is logged but the
 *    font record remains present (caller can decide how to handle).
 *  - Logs the load operation through the global `logger`.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `fonts` or ID state require
 *    external synchronization.
 *
 * Example:
 *  Font& f = rm.loadFont("Roboto", "assets/fonts/Roboto-Regular.ttf", 48);
 */
Font& ResourceManager::loadFont(const std::string& name, const std::string& filePath, unsigned int pixelSize, ResourceID id) {
	for (auto& [key, font] : fonts) {
		if (std::filesystem::path{ font.getFilePath() } == std::filesystem::path{ filePath } &&
			font.getPixelSize() == pixelSize) {
			return font;
		}
	}
	if (id == NEW_RESOURCE) id = getNewAssetID();

	auto [it, success] = fonts.try_emplace(id, id, name, filePath, pixelSize);
	assert(success);

	if (!it->second.LoadFont(filePath, pixelSize)) {
		logger.error("Failed to load font glyphs for: " + name + " from " + filePath);
	}

	logger.log("Loaded Font: " + name + " from " + filePath);
	return it->second;
}

/**
 * getFont
 *
 * Retrieve a previously loaded Font by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found Font object.
 *  - If no matching Font exists the function asserts (caller should ensure id
 *    validity) � consistent with existing usage in this codebase.
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  Font& f = rm.getFont(id); // assert if id not found
 */
Font& ResourceManager::getFont(ResourceID id) {
	auto found = fonts.find(id);
	assert(found != fonts.end());
	return found->second;
}

/**
 * loadPrefab
 *
 * Load or reuse a PrefabFile resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory PrefabFile record exists per file path. If a
 *    PrefabFile with an identical file path is already present, that
 *    PrefabFile is returned. Otherwise a new PrefabFile object is constructed
 *    and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the prefab (e.g. "PlayerPrefab").
 *  - filePath: Filesystem path to the prefab file.
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *
 * Returns:
 *  - Reference to the stored PrefabFile instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `prefabFiles` to detect an existing prefab by
 *    comparing std::filesystem::path values. This comparison is syntactic
 *    (string-based); it does not resolve symlinks or canonicalize paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts the new `PrefabFile` into `prefabFiles` and logs the operation.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `prefabFiles` or ID state
 *    require external synchronization.
 */
PrefabFile& ResourceManager::loadPrefab(const std::string& name, const std::string& filePath, ResourceID id) {
	for (auto& [key, prefabFile] : prefabFiles) {
		if (std::filesystem::path{ prefabFile.getFilePath() } == std::filesystem::path{ filePath }) {
			return prefabFile;
		}
	}

	if (id == NEW_RESOURCE) id = getNewAssetID();

	auto [it, success] = prefabFiles.emplace(id, PrefabFile{ id, name, filePath });
	assert(success);
	logger.log("Loaded Prefab: " + name + " from " + filePath);
	return it->second;
}

/**
 * getPrefab
 *
 * Retrieve a previously loaded PrefabFile by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found PrefabFile object.
 *  - If no matching PrefabFile exists the function asserts (caller should
 *    ensure id validity) � consistent with existing usage in this codebase.
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  PrefabFile& p = rm.getPrefab(id);
 */
PrefabFile& ResourceManager::getPrefab(ResourceID id) {
	auto found = prefabFiles.find(id);
	assert(found != prefabFiles.end());
	return found->second;
}

/**
 * loadScene
 *
 * Load or reuse a SceneFile resource record in the ResourceManager.
 *
 * Description:
 *  - Ensures a single in-memory SceneFile record exists per file path. If a
 *    SceneFile with an identical file path is already present, that
 *    SceneFile is returned. Otherwise a new SceneFile object is constructed
 *    and stored.
 *
 * Parameters:
 *  - name: Human-readable key for the scene (e.g. "Level1").
 *  - filePath: Filesystem path to the scene file.
 *  - id: Optional ResourceID to assign. If set to NEW_RESOURCE a new unique
 *        ResourceID is allocated via getNewAssetID().
 *  - startWithThisScene: If true, marks the SceneFile to indicate the engine
 *        should start with this scene (stored in SceneFile).
 *
 * Returns:
 *  - Reference to the stored SceneFile instance (existing or newly-inserted).
 *
 * Behavior & side-effects:
 *  - Performs a linear scan of `sceneFiles` to detect an existing scene by
 *    comparing std::filesystem::path values. This comparison is syntactic
 *    (string-based); it does not resolve symlinks or canonicalize paths.
 *  - If `id == NEW_RESOURCE`, a new unique ResourceID is generated and
 *    recorded in the ResourceManager's used ID registry.
 *  - Inserts the new `SceneFile` into `sceneFiles` and logs the operation.
 *
 * Thread-safety:
 *  - Not thread-safe. Concurrent calls that mutate `sceneFiles` or ID state
 *    require external synchronization.
 */
SceneFile& ResourceManager::loadScene(const std::string& name, const std::string& filePath, ResourceID id, bool startWithThisScene) {
	for (auto& [key, sceneFile] : sceneFiles) {
		if (std::filesystem::path{ sceneFile.getFilePath() } == std::filesystem::path{ filePath }) {
			return sceneFile;
		}
	}

	if (id == NEW_RESOURCE) id = getNewAssetID();
	sceneFiles.emplace(id, SceneFile{ id, name, filePath, startWithThisScene });
	logger.log("Loaded Scene: " + name + " from " + filePath);
	return sceneFiles.at(id);
}

/**
 * getScene
 *
 * Retrieve a previously loaded SceneFile by its ResourceID.
 *
 * Parameters:
 *  - id: the ResourceID to look up.
 *
 * Returns:
 *  - Reference to the found SceneFile object.
 *  - If no matching SceneFile exists the function asserts (caller should
 *    ensure id validity) � consistent with existing usage in this codebase.
 *
 * Simple usage:
 *  ResourceID id = /* previously obtained id * /;
 *  SceneFile& s = rm.getScene(id);
 */
SceneFile& ResourceManager::getScene(ResourceID id) {
	auto found = sceneFiles.find(id);
	assert(found != sceneFiles.end());
	return found->second;
}

/**
 * insertUsedID
 *
 * Marks a ResourceID as used in the ResourceManager's internal registry.
 * This is useful when restoring resource metadata (deserialization) so the
 * automatic ID allocator doesn't reissue IDs that are already present.
 *
 * Parameters:
 *  - id: reference to the ResourceID to insert into the used set.
 *
 * Side-effects:
 *  - Inserts `id` into `usedIDs`. If `id` already existed, a warning is logged.
 *
 * Simple usage:
 *  ResourceID idFromFile = 100;
 *  rm.insertUsedID(idFromFile); // prevents getNewAssetID() from returning 100
 */
void ResourceManager::insertUsedID(ResourceID& id) {
	auto [it, success] = usedIDs.insert(id);

	if (!success) {
		logger.warning("Trying to insert a used ResourceID: " + std::to_string(id));
	}
}

/**
 * getAudios
 *
 * Accessor that returns a mutable reference to the internal map of audios.
 * Use this to iterate or manipulate audios directly (careful: mutating keys
 * is undefined behavior; prefer ResourceManager operations).
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, Audio>
 *
 * Simple usage:
 *  auto& map = rm.getAudios();
 *  for (auto& [id, audio] : map) {  // inspect audio  }
 */
std::unordered_map<ResourceID, Audio>& ResourceManager::getAudios() {
	return audios;
}

/**
 * getShaders
 *
 * Accessor that returns a mutable reference to the internal map of shader files.
 * Use this to iterate or manipulate shader entries directly (careful: mutating
 * keys is undefined behavior; prefer ResourceManager operations).
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, ShaderFile>
 *
 * Simple usage:
 *  auto& map = rm.getShaders();
 *  for (auto& [id, shader] : map) {  // inspect shader  }
 */
std::unordered_map<ResourceID, ShaderFile>& ResourceManager::getShaders() {
	return shaderFiles;
}

/**
 * getTextures
 *
 * Accessor that returns a mutable reference to the internal map of texture files.
 * Use this to iterate or inspect textures stored in the ResourceManager.
 *
 * Notes:
 *  - The returned reference is mutable; callers must not modify map keys (undefined
 *    behavior) and should prefer ResourceManager methods for mutation when possible.
 *  - Not thread-safe: concurrent modifications require external synchronization.
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, TextureFile>
 *
 * Simple usage:
 *  auto& textures = rm.getTextures();
 *  for (auto& [id, tex] : textures) {
 *      // inspect tex
 *  }
 */
std::unordered_map<ResourceID, TextureFile>& ResourceManager::getTextures() {
	return textureFiles;
}

/**
 * getFonts
 *
 * Accessor that returns a mutable reference to the internal map of font resources.
 * Use this to iterate, inspect, or (carefully) mutate font entries directly.
 *
 * Notes:
 *  - The returned reference is mutable; callers must not modify map keys
 *    (undefined behavior). Prefer ResourceManager methods for insertion/removal.
 *  - Font entries are specific to a file path and pixel size; multiple entries
 *    for the same file at different pixel sizes can exist.
 *  - Not thread-safe: concurrent access that includes writers must be externally
 *    synchronized.
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, Font>
 *
 * Simple usage:
 *  auto& fonts = rm.getFonts();
 *  for (auto& [id, font] : fonts) {
 *      // inspect font metadata (do not change keys)
 *  }
 */
std::unordered_map<ResourceID, Font>& ResourceManager::getFonts() {
	return fonts;
}

/**
 * getPrefabs
 *
 * Accessor that returns a mutable reference to the internal map of prefab files.
 * Use this to iterate or manipulate prefab entries directly (careful: mutating
 * map keys is undefined behavior; prefer ResourceManager methods for mutation
 * when possible).
 *
 * Notes:
 *  - The returned reference is mutable; callers must not modify map keys.
 *  - Not thread-safe: concurrent modifications require external synchronization.
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, PrefabFile>
 *
 * Simple usage:
 *  auto& prefabs = rm.getPrefabs();
 *  for (auto& [id, prefab] : prefabs) {
 *      // inspect prefab
 *  }
 */
std::unordered_map<ResourceID, PrefabFile>& ResourceManager::getPrefabs() {
	return prefabFiles;
}

/**
 * getScenes
 *
 * Accessor that returns a mutable reference to the internal map of scene files.
 * Use this to iterate or manipulate scene entries directly (careful: mutating
 * map keys is undefined behavior; prefer ResourceManager methods for mutation
 * when possible).
 *
 * Notes:
 *  - The returned reference is mutable; callers must not modify map keys.
 *  - Not thread-safe: concurrent modifications require external synchronization.
 *
 * Returns:
 *  - Reference to std::unordered_map<ResourceID, SceneFile>
 *
 * Simple usage:
 *  auto& scenes = rm.getScenes();
 *  for (auto& [id, scene] : scenes) {
 *      // inspect scene
 *  }
 */
std::unordered_map<ResourceID, SceneFile>& ResourceManager::getScenes() {
	return sceneFiles;
}

/**
 * getScenes (const)
 *
 * Const accessor that returns a read-only reference to the internal map of
 * scene files. Use this when callers need to examine scenes without mutating
 * the ResourceManager state.
 *
 * Notes:
 *  - Safe for read-only access; still not thread-safe if concurrent writers
 *    exist. External synchronization is required for concurrent access patterns.
 *
 * Returns:
 *  - Const reference to std::unordered_map<ResourceID, SceneFile>
 *
 * Simple usage:
 *  const auto& scenes = rm.getScenes();
 *  if (scenes.find(id) != scenes.end()) { // inspect //  }
 */
const std::unordered_map<ResourceID, SceneFile>& ResourceManager::getScenes() const {
	return sceneFiles;
}

/**
 * getFilePathsForKey
 *
 * Collects the filesystem paths for all loaded resources of the given
 * `ResourceType` whose human-readable key equals `key`.
 *
 * Parameters:
 *  - type: The `ResourceType` enum value indicating which internal container
 *          to search (AUDIO, SHADER, TEXTURE, FONT, PREFAB, SCENE).
 *  - key:  The human-readable resource key to match (compared with
 *          `Resource::getKey()`).
 *
 * Returns:
 *  - std::vector<std::string> containing the `filePath` for every resource of
 *    the requested `type` where `resource.getKey() == key`. Returns an empty
 *    vector if no matches are found or if `type` is unsupported.
 *
 * Complexity:
 *  - O(n) in the number of loaded resources for the requested type (linear
 *    scan of the container).
 *
 * Notes:
 *  - The comparison is exact (case-sensitive); it does not canonicalize or
 *    resolve filesystem symlinks.
 *  - There are no side-effects; the function does not modify ResourceManager
 *    state.
 *  - Not thread-safe: concurrent writes to the underlying containers may
 *    invalidate iteration and must be externally synchronized.
 */
std::vector<std::string> ResourceManager::getFilePathsForKey(ResourceType type, const std::string& key) const {
	std::vector<std::string> filePaths;

	switch (type) {
		using enum ResourceType;
	case AUDIO:
		for (auto const& [id, file] : audios) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	case SHADER:
		for (auto const& [id, file] : shaderFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	case TEXTURE:
		for (auto const& [id, file] : textureFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	case FONT:
		for (auto const& [id, file] : fonts) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	case PREFAB:
		for (auto const& [id, file] : prefabFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	case SCENE:
		for (auto const& [id, file] : sceneFiles) {
			if (file.getKey() == key) filePaths.push_back(file.getFilePath());
		}
		break;
	default:
		break;
	}

	return filePaths;
}


/**
 * getNewAssetID
 *
 * Allocates and returns a new ResourceID that is not present in `usedIDs`.
 * The function advances `totalResourceID` until an unused value is found,
 * inserts it into `usedIDs`, and returns it.
 *
 * Returns:
 *  - A unique ResourceID for new resources.
 *
 * Simple usage:
 *  ResourceID newId = rm.getNewAssetID();
 *  // then use newId for newly-created resources
 * 
 */
ResourceID ResourceManager::getNewAssetID() {
	while (usedIDs.find(totalResourceID) != usedIDs.end()) {
		++totalResourceID;
	}

	usedIDs.insert(totalResourceID);
	return totalResourceID;
}

// for imgui assets browser to get textures
GLuint ResourceManager::getGLTextureFromFile(const std::string& filePath)
{
	// reuse texture
	auto it = loadedGLTextures.find(filePath);
	if (it != loadedGLTextures.end())
		return it->second->get_textureID(0);

	// load and store texture object
	std::vector<char const*> paths = { filePath.c_str() };
	auto tex = std::make_unique<Texture>(paths);

	if (tex->get_totalTexture() > 0)
	{
		GLuint texID = tex->get_textureID(0);
		loadedGLTextures[filePath] = std::move(tex); // store in manager
		logger.log("[ResourceManager] Cached persistent GL texture: " + filePath +
			" (ID " + std::to_string(texID) + ")");
		return texID;
	}

	logger.error("[ResourceManager] Failed to load texture: " + filePath);
	return 0;
}

void ResourceManager::clearAllGLTexturesCache() {
	loadedGLTextures.clear();
	logger.log("[ResourceManager] Cleared all cached GL textures.");
}
