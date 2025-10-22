#include <iostream>

#include "System/Audio_System.h"

/// AudioSystem
///  - Thin FMOD wrapper used by the engine to load/play/stop sounds.
///  - Use one instance, call init() before other calls, call update() each frame,
///    and call shutdown() when the engine closes.
///
/// See: ..\Engine\include\System\Audio_System.h for the public declarations.

AudioSystem::~AudioSystem() {
	shutdown();
}

/**
 * @brief Logs an FMOD error to the global logger if the result is not FMOD_OK.
 *
 * This is a private helper used by the AudioSystem to centralize FMOD error reporting.
 *
 * @param result FMOD_RESULT returned by an FMOD call.
 * @param context Short string describing the operation (e.g. "FMOD_System_Init", "PlaySound SFX: key").
 *
 * Usage:
 *  - Internal only. Example (internal):
 *      logFMODError(result, "CreateSound: assets/sfx.wav");
 */
void AudioSystem::logFMODError(FMOD_RESULT result, const std::string& context) {
	if (result != FMOD_OK) {
		const char* errorMessage = FMOD_ErrorString(result);
		logger.error(std::string("FMOD Error in (") + context + "): " + errorMessage);
	}
}

/**
 * @brief Initialize the FMOD system.
 *
 * Creates the FMOD system object and initializes it with a default number of channels.
 * Safe to call multiple times; subsequent calls will return true if already initialized.
 *
 * @return true on successful initialization, false on error.
 *
 * Usage:
 *  AudioSystem audio;
 *  if (!audio.init()) {
 *      // handle failure (logger contains details)
 *  }
 */
bool AudioSystem::init() {
	if (fmodSystem) return true;

	FMOD_RESULT result = FMOD_System_Create(&fmodSystem, FMOD_VERSION);
	if (result != FMOD_OK) {
		logFMODError(result, "FMOD_System_Create");
		return false;
	}

	result = FMOD_System_Init(fmodSystem, 512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		logFMODError(result, "FMOD_System_Init");
		FMOD_System_Release(fmodSystem);
		fmodSystem = nullptr;
		return false;
	}

	logger.log("AudioSystem initialized successfully.");
	return true;
}

/**
 * @brief Update the FMOD system.
 *
 * Must be called regularly (typically once per frame) so FMOD can perform internal updates
 * (streaming, voice management, DSP, etc.). This call is inexpensive and non-blocking.
 *
 * Usage (per-frame):
 *  audio.update();
 */
void AudioSystem::update() {
	if (!fmodSystem) return;
	FMOD_System_Update(fmodSystem);
}

/**
 * @brief Shutdown the audio system and release all loaded resources.
 *
 * Stops BGM, releases all FMOD_Sound objects, closes and releases the FMOD system.
 * Safe to call multiple times.
 *
 * Usage:
 *  audio.shutdown();
 */
void AudioSystem::shutdown() {
	if (!fmodSystem) return;

	if (bgmChannel) {
		FMOD_Channel_Stop(bgmChannel);
		bgmChannel = nullptr;
	}

	for (auto& [key, sound] : soundMap) {
		if (sound) {
			FMOD_Sound_Release(sound);
		}
	}

	soundMap.clear();

	FMOD_System_Close(fmodSystem);
	FMOD_System_Release(fmodSystem);
	fmodSystem = nullptr;

	logger.log("AudioSystem shutdown successfully.");
}

/**
 * @brief Load a sound into the audio system.
 *
 * Loads a sound from disk (or creates a stream) and stores it in an internal map under the provided key.
 * If the key is already loaded this function will warn and return true.
 *
 * @param key   Identifier used later to play the sound (must be unique).
 * @param path  Filesystem path to the audio asset (e.g. "assets/sfx.wav").
 * @param stream When true, the asset is streamed from disk (good for large music files).
 *               When false, the asset is loaded as a sample (good for short SFX).
 * @param loop  When true, the sound will be created with looping enabled (FMOD_LOOP_NORMAL).
 *
 * @return true on success, false on error (init must have been called first).
 *
 * Usage:
 *  // Load background music as a stream and loop it:
 *  audio.loadSound("bgm_main", "assets/music/theme.ogg", true, true);
 *
 *  // Load a short SFX sample:
 *  audio.loadSound("explosion", "assets/sfx/explosion.wav", false, false);
 */
bool AudioSystem::loadSound(const std::string& key, const std::string& path, bool stream, bool loop) {
	if (!fmodSystem) {
		logger.error("AudioSystem::loadSound called before init");
		return false;
	}

	auto it = soundMap.find(key);
	if (it != soundMap.end()) {
		logger.warning("AudioSystem::loadSound: key already loaded: " + key);
		return true;
	}

	FMOD_SOUND* sound = nullptr;
	FMOD_RESULT result;
	FMOD_MODE mode = FMOD_DEFAULT;

	if (stream) {
		mode |= FMOD_CREATESTREAM;
	}
	else {
		mode |= FMOD_CREATESAMPLE;
	}

	if (loop) {
		mode |= FMOD_LOOP_NORMAL;
	}
	else {
		mode |= FMOD_LOOP_OFF;
	}

	if (stream) {
		result = FMOD_System_CreateStream(fmodSystem, path.c_str(), mode, nullptr, &sound);
	}
	else {
		result = FMOD_System_CreateSound(fmodSystem, path.c_str(), mode, nullptr, &sound);
	}

	if (result != FMOD_OK) {
		logFMODError(result, "CreateSound/CreateStream: " + path);
		return false;
	}

	soundMap.emplace(key, sound);
	logger.log("AudioSystem::loadSound: Loaded sound: " + key + " from " + path);
	return true;
}

/**
 * @brief Play background music (BGM) previously loaded with loadSound.
 *
 * Plays the sound associated with `key` on the BGM channel. If a BGM is already playing it will be replaced.
 * The volume parameter is multiplied by the configured BGM master volume (set via setBGMVolume).
 *
 * @param key Identifier used when the sound was loaded.
 * @param volume Per-call volume multiplier in range [0.0, 1.0] (default 1.0).
 *
 * @return true on success, false if the sound isn't loaded or FMOD reports an error.
 *
 * Usage:
 *  audio.playBGM("bgm_main", 0.8f);
 *  // To stop:
 *  audio.stopBGM();
 */
bool AudioSystem::playBGM(const std::string& key, float volume) {
	if (!fmodSystem) {
		logger.error("AudioSystem::playBGM called before init");
		return false;
	}

	auto it = soundMap.find(key);
	if (it == soundMap.end()) {
		logger.error("AudioSystem::playBGM: sound not found: " + key);
		return false;
	}

	FMOD_SOUND* sound = it->second;
	FMOD_RESULT result = FMOD_System_PlaySound(fmodSystem, sound, nullptr, true, &bgmChannel);
	if (result != FMOD_OK) {
		logFMODError(result, "PlaySound BGM: " + key);
		return false;
	}

	if (bgmChannel) {
		FMOD_Channel_SetVolume(bgmChannel, volume * bgmVolume);
		FMOD_Channel_SetPaused(bgmChannel, false);
	}

	logger.log("AudioSystem::playBGM: Playing BGM: " + key);
	return true;
}

/**
 * @brief Stop the currently playing BGM (if any).
 *
 * Usage:
 *  audio.stopBGM();
 */
void AudioSystem::stopBGM() {
	if (!bgmChannel) return;
	FMOD_Channel_Stop(bgmChannel);
	bgmChannel = nullptr;
}

/**
 * @brief Play a short sound effect (SFX) previously loaded with loadSound.
 *
 * This will create a temporary channel for the SFX so multiple SFX can overlap.
 * The per-call volume is multiplied by the SFX master volume (set via setSFXVolume).
 *
 * @param key Identifier used when the sound was loaded.
 * @param volume Per-call volume multiplier in range [0.0, 1.0] (default 1.0).
 *
 * @return true on success, false on error or if the sound isn't loaded.
 *
 * Usage:
 *  audio.playSFX("explosion", 0.9f);
 */
bool AudioSystem::playSFX(const std::string& key, float volume) {
	if (!fmodSystem) {
		logger.error("AudioSystem::playSFX called before init");
		return false;
	}

	auto it = soundMap.find(key);
	if (it == soundMap.end()) {
		logger.error("AudioSystem::playSFX: sound not found: " + key);
		return false;
	}

	FMOD_SOUND* sound = it->second;
	FMOD_CHANNEL* sfxChannel = nullptr;
	FMOD_RESULT result = FMOD_System_PlaySound(fmodSystem, sound, nullptr, false, &sfxChannel);
	if (result != FMOD_OK) {
		logFMODError(result, "PlaySound SFX: " + key);
		return false;
	}

	if (sfxChannel) {
		FMOD_Channel_SetVolume(sfxChannel, volume * sfxVolume);
	}

	logger.log("AudioSystem::playSFX: Playing SFX: " + key);
	return true;
}

/**
 * @brief Set the master background music volume.
 *
 * Volume is clamped to [0.0, 1.0]. If BGM is currently playing the channel's volume is updated.
 *
 * @param volume New volume in range [0.0, 1.0].
 *
 * Usage:
 *  audio.setBGMVolume(0.75f);
 */
void AudioSystem::setBGMVolume(float volume) {
	bgmVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
	if (bgmChannel) {
		FMOD_Channel_SetVolume(bgmChannel, bgmVolume);
	}
}

/**
 * @brief Set the master SFX volume.
 *
 * Volume is clamped to [0.0, 1.0]. New SFX channels will multiply this value against per-call volumes.
 *
 * @param volume New volume in range [0.0, 1.0].
 *
 * Usage:
 *  audio.setSFXVolume(0.6f);
 */
void AudioSystem::setSFXVolume(float volume) {
	sfxVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}
