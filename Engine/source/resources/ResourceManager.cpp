// Add filepath here
constexpr char DEFAULT_AUDIO_FILEPATH[] = "DefaultAudio/filepath";

#include <iostream>
#include <filesystem>
#include "fmod_errors.h"

#include "ECS/ECS.h"
#include "resources/ResourceManager.h"
#include "core/Logger.h"
#include "System/Audio_System.h"

extern Logger logger;

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
	: defaultAudio{ 0, "Default", DEFAULT_AUDIO_FILEPATH }, ecs{ ecs }, audioSystem{ audioSystem } {
	// Deserialize the asset meta data here (TODO)
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
	// Serialize the asset meta data here (TODO)
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
	logger.log("Beginning resource loading...");

	std::string resourcePath{ "assets" };
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
		if (resourceFilePath == "audio") {
			loadAudio(stem.string(), path.string());
		}
		else if (resourceFilePath == "shaders") {
			loadShader(stem.string(), path.string());
		}
	}

	logger.log("Resource loading complete.");
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
		return defaultAudio;
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
 *    ensure id validity) — consistent with existing usage in this codebase.
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
 */
ResourceID ResourceManager::getNewAssetID() {
	while (usedIDs.find(totalResourceID) != usedIDs.end()) {
		++totalResourceID;
	}

	usedIDs.insert(totalResourceID);
	return totalResourceID;
}
