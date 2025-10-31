#include "System/Audio_System.h"
#include <filesystem>

namespace {
	/**
	 * @brief Pointer to the currently active AudioSystem instance.
	 *
	 * This file-scoped pointer is used by FMOD channel callbacks to route
	 * events back to the owning AudioSystem instance. It is assigned in
	 * AudioSystem::AudioSystem and cleared on destruction.
	 *
	 * Usage:
	 * - Do not access this pointer directly outside this translation unit.
	 */
	AudioSystem* audioSystem = nullptr;

	/**
	 * @brief FMOD channel callback used to detect when a channel finishes playback.
	 *
	 * FMOD will call this callback for channel/channelgroup events. This callback
	 * checks for FMOD_CHANNELCONTROL_CALLBACK_END and forwards completed-channel
	 * events to the active AudioSystem by calling handleCompletedAudioInstance.
	 *
	 * Parameters correspond to FMOD's signature. Only FMOD_CHANNELCONTROL_CALLBACK_END
	 * is handled; other callback types return FMOD_OK without action.
	 *
	 * @param channelControl Raw FMOD channel/channelgroup pointer.
	 * @param controlType The type of control (channel or channelgroup).
	 * @param callbackType The callback event type.
	 * @param commandData1 Unused (provided by FMOD).
	 * @param commandData2 Unused (provided by FMOD).
	 * @return FMOD_OK always (errors are logged by AudioSystem itself).
	 *
	 * Example:
	 * - This callback is installed per-channel in `createAudioInstance` via `channel->setCallback`.
	 */
	FMOD_RESULT channelCallback (FMOD_CHANNELCONTROL* channelControl,
								FMOD_CHANNELCONTROL_TYPE controlType,
								FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
								void* commandData1,
								void* commandData2) {
		(void)commandData1;
		(void)commandData2;

		if (callbackType != FMOD_CHANNELCONTROL_CALLBACK_END) {
			return FMOD_OK;
		}

		switch (controlType) {
		/*case FMOD_CHANNELCONTROL_CHANNEL: {
			FMOD::Channel* channel = (FMOD::Channel*)channelControl;
			audioSystem->handleCompletedAudioInstance(channel);
			return FMOD_OK;
		}*/

		case FMOD_CHANNELCONTROL_CHANNEL: {
			FMOD::Channel* channel = (FMOD::Channel*)channelControl;
			if (audioSystem) { // guard in case FMOD fires after shutdown
				audioSystem->handleCompletedAudioInstance(channel);
			}
			return FMOD_OK;
		}

		case FMOD_CHANNELCONTROL_CHANNELGROUP: {
			return FMOD_OK;
		}
		default:
			return FMOD_OK;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Construct an AudioSystem and initialize FMOD resources.
 *
 * Initializes the FMOD system, creates channel groups (Master, BGM, SFX, Reserved1,
 * Reserved2), sets up a low-pass DSP (initially bypassed), and preloads any audio
 * resources present in the ResourceManager.
 *
 * @param name Human-readable system name (forwarded to System base).
 * @param ecs Reference to the ECS registry used by System.
 * @param rm Reference to the ResourceManager which stores audio resources.
 *
 * Notes:
 * - Logs errors and returns early on FMOD failures; the AudioSystem instance may
 *   exist but be partially initialized if FMOD calls fail.
 *
 * Simple usage:
 * - Constructed by engine initialization code. No direct calls needed from gameplay.
 */
AudioSystem::AudioSystem(std::string name, ECS& ecs, ResourceManager& rm) : System{ ecs, name }, resourceManager{ rm }, bgmVolume{ 1.0f }, sfxVolume{ 1.0f } {
	audioSystem = this;

	if (FMOD::System_Create(&fmodSystem) != FMOD_OK) {
		logger.error("Failed to create FMOD system");
		return;
	}

	if (fmodSystem->init(512, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
		logger.error("Failed to initialize FMOD system");
		return;
	}

	if (fmodSystem->createChannelGroup("MasterChannelGroup", &masterChannelGroup) != FMOD_OK ||
		fmodSystem->createChannelGroup("BGM", &bgmChannelGroup) != FMOD_OK ||
		fmodSystem->createChannelGroup("SFX", &sfxChannelGroup) != FMOD_OK ||
		fmodSystem->createChannelGroup("Reserve1", &reserved1ChannelGroup) != FMOD_OK ||
		fmodSystem->createChannelGroup("Reserve2", &reserved2ChannelGroup) != FMOD_OK) {

		logger.error("Failed to create FMOD channel group");
		return;
	}

	masterChannelGroup->addGroup(bgmChannelGroup);
	masterChannelGroup->addGroup(sfxChannelGroup);
	masterChannelGroup->addGroup(reserved1ChannelGroup);
	masterChannelGroup->addGroup(reserved2ChannelGroup);

	if (fmodSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPassFilter) != FMOD_OK) {
		logger.error("Failed to create FMOD low pass filter DSP");
	}

	lowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, 500.0f); // default cutoff frequency
	bgmChannelGroup->addDSP(0, lowPassFilter);
	lowPassFilter->setBypass(true); // start disabled

	for (auto& [id, audio] : resourceManager.getAudios()) {
		loadSound(audio);
	}

	//FMOD::Sound* defaultSound = nullptr;
	//FMOD_RESULT result = fmodSystem->createSound(resourceManager.defaultAudio.getFilePath().c_str(), false, nullptr, &defaultSound);
	
	FMOD::Sound* defaultSound = nullptr;
	// Use the accessor + a valid FMOD_MODE flag instead of 'false'
	FMOD_RESULT result = fmodSystem->createSound(
		resourceManager.getDefaultAudio().getFilePath().c_str(),
		FMOD_2D, nullptr, &defaultSound
	);


	if (result != FMOD_OK) {
		logger.error("Failed to load default audio sound" + std::string{ FMOD_ErrorString(result) });
	}
	soundMap.insert({ 0, defaultSound });

	setBGMVolume(bgmVolume);
	setSFXVolume(sfxVolume);
}

/**
 * @brief Releases FMOD resources and stops all audio.
 *
 * Releases loaded sounds, DSPs, channel groups and the FMOD system. After
 * destruction the global callback pointer used by FMOD remains invalid.
 *
 * Simple usage:
 * - Called automatically during engine shutdown.
 */
AudioSystem::~AudioSystem() {
	for (auto& [id, sound] : soundMap) {
		if (sound) {
			sound->release();
			sound = nullptr;
		}
	}

	if (lowPassFilter) {
		lowPassFilter->release();
		lowPassFilter = nullptr;
	}

	if (masterChannelGroup) {
		masterChannelGroup->release();
		masterChannelGroup = nullptr;
	}

	if (bgmChannelGroup) {
		bgmChannelGroup->release();
		bgmChannelGroup = nullptr;
	}

	if (sfxChannelGroup) {
		sfxChannelGroup->release();
		sfxChannelGroup = nullptr;
	}

	if (reserved1ChannelGroup) {
		reserved1ChannelGroup->release();
		reserved1ChannelGroup = nullptr;
	}

	if (reserved2ChannelGroup) {
		reserved2ChannelGroup->release();
		reserved2ChannelGroup = nullptr;
	}

	if (fmodSystem) {
		fmodSystem->close();
		fmodSystem->release();
		fmodSystem = nullptr;
	}

	audioSystem = nullptr; // clear file-scoped pointer used by the callback

}

////////////////////////////////////////////////////////////////////////////////
// Main update tick
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Update the FMOD system and internal audio instance state.
 *
 * Must be called once per frame. Calls `fmodSystem->update()` and:
 * - Removes audio instances flagged for destruction.
 * - Applies per-instance volume values to active channels.
 *
 * @param deltaTime Frame delta time in seconds (currently unused).
 *
 * Simple usage:
 * - Called from the engine's main loop once per frame.
 */
void AudioSystem::update(double deltaTime) {
	if (fmodSystem) {
		fmodSystem->update();
	}

	for (auto it = audioInstances.begin(); it != audioInstances.end(); ) {
		auto& inst = it->second;

		if (inst.toDestroy) {
			if (inst.channel) inst.channel->stop();
			it = audioInstances.erase(it);
			continue;
		}

		if (inst.channel) inst.channel->setVolume(inst.volume);
		++it;
	}

}

////////////////////////////////////////////////////////////////////////////////
// Audio instance lifecycle
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Create and start an audio instance from an audio resource.
 *
 * This function looks up the `FMOD::Sound*` for `audioData.resourceID`, creates
 * a channel and configures it (volume, loop mode, channel group, callback).
 * The newly-created instance is stored in `audioInstances` and its ID returned.
 *
 * @param audioData Per-instance playback data (volume, loop, group, persistence).
 * @param id Parent entity ID that owns this audio instance (may be INVALID_ENTITY_ID).
 * @return AudioInstanceID on success, std::nullopt on failure.
 *
 * Failure cases:
 * - Missing sound resource in `soundMap`.
 * - FMOD failed to create or start the channel.
 *
 * Example:
 * AudioData::IndividualAudioData soundData{resourceID=5, volume=0.8f, loop=false, group=AudioData::IndividualAudioData::Group::SFX, ...};
 * auto optId = audioSystem.createAudioInstance(soundData, myEntityId);
 * if (optId) { // playback started  }
 */
std::optional<AudioInstanceID> AudioSystem::createAudioInstance(AudioData::IndividualAudioData const& audioData, EntityID id) {
	ResourceID const& audioResourceID = audioData.resourceID;

	auto soundIt = soundMap.find(audioResourceID);
	if (soundIt == soundMap.end()) {
		logger.error("Failed to find audio resource with ID: " + std::to_string(audioResourceID));
		return std::nullopt;
	}

	FMOD::Sound* sound = soundIt->second;
	FMOD::Channel* channel = nullptr;

	if (fmodSystem->playSound(sound, nullptr, false, &channel) != FMOD_OK) {
		logger.error("Failed to play sound for audio resource ID: " + std::to_string(audioResourceID));
		return std::nullopt;
	}

	AudioInstance newAudioInstance{ ++totalInstanceCount, channel, true, audioData.volume,
		audioData.persistOnEntityDestruction, audioData.persistOnSceneChange, id };

	float volume = audioData.volume;

	channel->setVolume(volume);
	channel->setCallback(channelCallback);
	channel->setMode(audioData.loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);

	switch (audioData.group) {
	case AudioData::IndividualAudioData::Group::Music:
		channel->setChannelGroup(bgmChannelGroup);
		break;
	case AudioData::IndividualAudioData::Group::SFX:
		channel->setChannelGroup(sfxChannelGroup);
		break;
	case AudioData::IndividualAudioData::Group::RESERVED1:
		channel->setChannelGroup(reserved1ChannelGroup);
		break;
	case AudioData::IndividualAudioData::Group::RESERVED2:
		channel->setChannelGroup(reserved2ChannelGroup);
		break;
	}

	audioInstances[newAudioInstance.instanceID] = newAudioInstance;
	return newAudioInstance.instanceID;
}

/**
 * @brief Restart playback of an existing audio instance.
 *
 * Sets the playback position to 0 ms. Does not automatically un-pause the channel.
 *
 * @param id AudioInstanceID returned from `createAudioInstance`.
 *
 * Example:
 * audioSystem.restartSound(instanceId);
 */
void AudioSystem::restartSound(AudioInstanceID id) {
	auto it = audioInstances.find(id);
	if (it == audioInstances.end()) {
		logger.warning("Attempted to restart non-existent audio instance with ID: " + std::to_string(id));
		return;
	}

	it->second.channel->setPosition(0, FMOD_TIMEUNIT_MS);
}

/**
 * @brief Resume a paused audio instance.
 *
 * Un-pauses the FMOD channel and validates the channel state. If the underlying
 * channel is invalid, the instance will be destroyed.
 *
 * @param id AudioInstanceID to resume.
 * @return true if resumed successfully, false if the instance was invalid or could not be resumed.
 *
 * Example:
 * if (!audioSystem.resumeSound(instanceId)) { // handle failure //  }
 */
bool AudioSystem::resumeSound(AudioInstanceID id) {
	auto it = audioInstances.find(id);
	if (it == audioInstances.end()) {
		logger.warning("Attempted to resume non-existent audio instance with ID: " + std::to_string(id));
		return false;
	}

	FMOD::Channel* channel = it->second.channel;
	channel->setPaused(false);

	bool isPaused;
	channel->getPaused(&isPaused);

	if (isPaused) {
		logger.warning("Attempting to resume audio instance that is no longer valid");
		destroyAudioInstance(id);
		return false;
	}

	it->second.isPlaying = true;
	return true;
}

/**
 * @brief Pause a playing audio instance.
 *
 * Marks instance as not playing and pauses the FMOD channel.
 *
 * @param id AudioInstanceID to pause.
 */
void AudioSystem::pauseSound(AudioInstanceID id) {
	auto it = audioInstances.find(id);
	if (it == audioInstances.end()) {
		logger.error("Attempted to pause non-existent audio instance with ID: " + std::to_string(id));
		return;
	}

	it->second.channel->setPaused(true);
	it->second.isPlaying = false;
}

/**
 * @brief Modify the volume of a single audio instance.
 *
 * Clamps `val` to [0.0f, 1.0f], updates the stored instance volume and applies it
 * to the FMOD channel if available.
 *
 * @param id AudioInstanceID to modify.
 * @param val New volume value in range [0.0, 1.0].
 *
 * Example:
 * audioSystem.modifyVolume(instanceId, 0.5f);
 */
void AudioSystem::modifyVolume(AudioInstanceID id, float val) {
	auto it = audioInstances.find(id);
	if (it != audioInstances.end()) {
		val = std::clamp(val, 0.0f, 1.0f);
		it->second.volume = val;

		if (it->second.channel) {
			it->second.channel->setVolume(val);
		}
	}
}

/**
 * @brief Retrieve the stored volume for a given audio instance.
 *
 * @param id AudioInstanceID to query.
 * @return Stored volume value [0.0, 1.0]. Returns 0.0f if instance not found.
 */
float AudioSystem::getVolume(AudioInstanceID id) {
	auto it = audioInstances.find(id);
	if (it == audioInstances.end()) {
		logger.error("Attempted to get volume of non-existent audio instance with ID: " + std::to_string(id));
		return 0.0f;
	}
	return it->second.volume;
}

/**
 * @brief Pause all audio routed through the SFX channel group.
 *
 * Useful for temporarily muting sound effects while leaving music playing.
 *
 * Example:
 * audioSystem.pauseAllSFX();
 */
void AudioSystem::pauseAllSFX() {
	if (sfxChannelGroup) {
		sfxChannelGroup->setPaused(true);
	}
}

/**
 * @brief Resume playback of the SFX channel group.
 *
 * Example:
 * audioSystem.resumeAllSFX();
 */
void AudioSystem::resumeAllSFX() {
		if (sfxChannelGroup) {
		sfxChannelGroup->setPaused(false);
		}
}

/**
 * @brief Get the raw FMOD system pointer.
 *
 * Use with care; prefer AudioSystem methods for common operations.
 *
 * @return Pointer to `FMOD::System` or nullptr if not initialized.
 */
FMOD::System* AudioSystem::getSystem() const {
	return fmodSystem;
}

/**
 * @brief Stop and remove a single audio instance immediately.
 *
 * If the instance does not exist a warning is logged.
 *
 * @param instanceID ID returned by `createAudioInstance`.
 */
void AudioSystem::destroyAudioInstance(AudioInstanceID instanceID) {
	auto it = audioInstances.find(instanceID);
	if (it == audioInstances.end()) {
		logger.warning("Attempted to destroy non-existent audio instance with ID: " + std::to_string(instanceID));
		return;
	}
	it->second.channel->stop();
	audioInstances.erase(it);
}

/**
 * @brief Handle cleanup or persistence when an entity is destroyed.
 *
 * Iterates `audioInstances` and:
 * - If the instance belongs to `entityId` and `persistAfterEntityDestruction` is true:
 *     - Clears the parentEntityID so the sound becomes unowned and continues playing.
 * - Otherwise stops and removes the audio instance.
 *
 * @param entityId EntityID being destroyed.
 */
void AudioSystem::handleEntityDestruction(EntityID entityId) {
	for (auto it = audioInstances.begin(); it != audioInstances.end();) {
		if (it->second.parentEntityID == entityId) {
			if (it->second.persistAfterEntityDestruction) {
				it->second.parentEntityID = INVALID_ENTITY_ID;
				++it;
			}
			else {
				it->second.channel->stop();
				it = audioInstances.erase(it);
			}
		}
		else {
			++it;
		}
	}
}

/**
 * @brief Mark an audio instance for destruction when FMOD signals playback end.
 *
 * This function is invoked by the FMOD channel callback. It finds the matching
 * AudioInstance by comparing the channel pointer and sets `toDestroy` so the
 * next `update` call can remove it safely.
 *
 * @param channel The FMOD channel that completed playback.
 */
void AudioSystem::handleCompletedAudioInstance(FMOD::Channel* channel) {
	auto it = std::find_if(audioInstances.begin(), audioInstances.end(), [&](auto const& keyPairValue) {
		return channel == keyPairValue.second.channel;
		});

	if (it == audioInstances.end()) {
		return;
	}

	it->second.toDestroy = true;
}

/**
 * @brief Remove all audio instances, optionally preserving those marked persistent.
 *
 * @param forceDeletePersistentSounds If true, ignores `persistOnSceneChange` and deletes all instances.
 *
 * Example:
 * // Called when loading a new scene:
 * audioSystem.deleteAllAudioInstances(false);
 */
void AudioSystem::deleteAllAudioInstances(bool forceDeletePersistentSounds) {
	for (auto it = audioInstances.begin(); it != audioInstances.end();) {
		if (it->second.persistOnSceneChange && !forceDeletePersistentSounds) {
			++it;
			continue;
		}

		it->second.channel->stop();
		it = audioInstances.erase(it);
	}
}

/**
 * @brief Called when a new AudioData component is registered for an entity.
 *
 * For each `IndividualAudioData` in `audioData.audios` that is flagged `toPlayAtStart`,
 * creates an audio instance attached to `id`. If any requested start fails, existing
 * created instances are deleted and the function returns false.
 *
 * @param audioData AudioData component describing audios to register.
 * @param id EntityID to associate with created audio instances.
 * @return true if all requested startup playbacks succeeded; false otherwise.
 */
bool AudioSystem::registerNewAudioResource(AudioData& audioData, EntityID id) {
	for (auto& audio : audioData.audios) {
		if (!audio.toPlayAtStart) {
			continue;
		}

		if (!createAudioInstance(audio, id)) {
			logger.error("Failed to create audio instance for entity ID: " + std::to_string(id));
			deleteAllAudioInstances();
			return false;
		}
	}
	return true;
}

/**
 * @brief Load an audio file into an FMOD::Sound and store it in `soundMap`.
 *
 * Uses `FMOD_2D` by default. On failure an error is logged and the sound is not inserted.
 *
 * @param audio Reference to `Audio` resource containing file path and id.
 *
 * Example:
 * // Called by the ResourceManager during initialization; not typically called by gameplay code.
 */
void AudioSystem::loadSound(Audio const& audio) {
	FMOD::Sound* sound = nullptr;
	//FMOD_RESULT result = fmodSystem->createSound(audio.getFilePath().c_str(), FMOD_2D, nullptr, &sound);
	FMOD_MODE mode = FMOD_DEFAULT | FMOD_2D | FMOD_CREATESTREAM; // stream long files
	FMOD_RESULT result = fmodSystem->createSound(audio.getFilePath().c_str(), mode, nullptr, &sound);
	if (result != FMOD_OK) {
		logger.error("Failed to load audio sound from file: " + audio.getFilePath() + " Error: " + std::string{ FMOD_ErrorString(result) });
		return;
	}

	soundMap.insert({ audio.getID(), sound });
}

/**
 * @brief Enable or disable the low-pass DSP applied to the BGM channel group.
 *
 * @param enable true to apply the low-pass, false to bypass it.
 *
 * Example:
 * // Apply a muffled effect:
 * audioSystem.toggleLowPassFilter(true);
 */
void AudioSystem::toggleLowPassFilter(bool enable) {
	if (enable) {
		lowPassFilter->setBypass(false);
	}
	else {
		lowPassFilter->setBypass(true);
	}
}

/**
 * @brief Set the master channel group's volume (clamped to [0,1]).
 *
 * @param val Desired master volume.
 */
void AudioSystem::setMasterVolume(float val) {
	val = std::clamp(val, 0.0f, 1.0f);
	if (masterChannelGroup) {
		masterChannelGroup->setVolume(val);
	}
}

/**
 * @brief Set the SFX channel group's volume (clamped to [0,1]).
 *
 * @param val Desired SFX volume.
 */
void AudioSystem::setSFXVolume(float val) {
	sfxVolume = std::clamp(val, 0.0f, 1.0f);
	if (sfxChannelGroup) {
		sfxChannelGroup->setVolume(sfxVolume);
	}
}

/**
 * @brief Set the BGM channel group's volume (clamped to [0,1]).
 *
 * @param val Desired BGM volume.
 */
void AudioSystem::setBGMVolume(float val) {
	bgmVolume = std::clamp(val, 0.0f, 1.0f);
	if (bgmChannelGroup) {
		bgmChannelGroup->setVolume(bgmVolume);
	}
}

/**
 * @brief Set Reserved1 channel group's volume (clamped to [0,1]).
 */
void AudioSystem::setReserved1Volume(float val) {
	val = std::clamp(val, 0.0f, 1.0f);
	if (reserved1ChannelGroup) {
		reserved1ChannelGroup->setVolume(val);
	}
}

/**
 * @brief Set Reserved2 channel group's volume (clamped to [0,1]).
 */
void AudioSystem::setReserved2Volume(float val) {
	val = std::clamp(val, 0.0f, 1.0f);
	if (reserved2ChannelGroup) {
		reserved2ChannelGroup->setVolume(val);
	}
}

/**
 * @brief Query master channel group's current volume.
 *
 * @return Current master volume in range [0,1]. If group is null returns 0.0f.
 */
float AudioSystem::getMasterVolume() const {
	float volume = 0.0f;
	masterChannelGroup->getVolume(&volume);
	return volume;
}

/**
 * @brief Query SFX channel group's current volume.
 *
 * @return Current SFX volume in range [0,1]. If group is null returns 0.0f.
 */
float AudioSystem::getSFXVolume() const {
	float volume = 0.0f;
	sfxChannelGroup->getVolume(&volume);
	return volume;
}

/**
 * @brief Query BGM channel group's current volume.
 *
 * @return Current BGM volume in range [0,1]. If group is null returns 0.0f.
 */
float AudioSystem::getBGMVolume() const {
	float volume = 0.0f;
	bgmChannelGroup->getVolume(&volume);
	return volume;
}

/**
 * @brief Query Reserved1 channel group's current volume.
 */
float AudioSystem::getReserved1Volume() const {
	float volume = 0.0f;
	reserved1ChannelGroup->getVolume(&volume);
	return volume;
}

/**
 * @brief Query Reserved2 channel group's current volume.
 */
float AudioSystem::getReserved2Volume() const {
	float volume = 0.0f;
	reserved2ChannelGroup->getVolume(&volume);
	return volume;
}

/**
 * @brief Play a background music sound by resource ID.
 *
 * Looks up the preloaded `FMOD::Sound*` for `id` in `soundMap` and starts playback
 * routed to the BGM channel group. The created FMOD channel is stored in an
 * `AudioInstance` and an `AudioInstanceID` is returned.
 *
 * Behavior:
 * - If the resource is not found the function logs an error and returns 0.
 * - If FMOD fails to start playback the function logs an error and returns 0.
 * - The new instance is created with `isPlaying = true` and its volume initialized
 *   from the current `bgmVolume`.
 *
 * Notes:
 * - This function does not configure looping, persistence, or callbacks; use
 *   `createAudioInstance` when per-instance configuration is required.
 *
 * @param id ResourceID of the audio asset to play as BGM.
 * @return AudioInstanceID ID of the created audio instance on success, or 0 on failure.
 */
AudioInstanceID AudioSystem::playBGM(ResourceID id) {
	auto it = soundMap.find(id);
	if (it == soundMap.end()) {
		logger.error("AudioSystem::playBGM: Failed to find audio resource with ID: " + std::to_string(id));
		return 0;
	}

	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = fmodSystem->playSound(it->second, bgmChannelGroup, false, &channel);
	if (result != FMOD_OK) {
		logger.error("AudioSystem::playBGM: Failed to play BGM sound for audio resource ID: " + std::to_string(id));
		return 0;
	}

	AudioInstance inst{};
	inst.channel = channel;
	inst.isPlaying = true;
	inst.volume = bgmVolume;
	++totalInstanceCount;
	audioInstances[totalInstanceCount] = std::move(inst);

	channel->setVolume(bgmVolume);
	return totalInstanceCount;
}

/**
 * @brief Play a sound effect (SFX) by resource ID.
 *
 * Looks up the preloaded `FMOD::Sound*` for `id` in `soundMap` and starts playback
 * routed to the SFX channel group. The created FMOD channel is stored in an
 * `AudioInstance` and an `AudioInstanceID` is returned.
 *
 * Behavior:
 * - If the resource is not found the function logs an error and returns 0.
 * - If FMOD fails to start playback the function logs an error and returns 0.
 * - The new instance is created with `isPlaying = true` and its volume initialized
 *   from the current `sfxVolume`.
 *
 * Notes:
 * - This function is intended for simple, fire-and-forget SFX playback. For more
 *   controlled playback (looping, persistence, callbacks) prefer `createAudioInstance`.
 *
 * @param id ResourceID of the audio asset to play as SFX.
 * @return AudioInstanceID ID of the created audio instance on success, or 0 on failure.
 */
AudioInstanceID AudioSystem::playSFX(ResourceID id) {
	auto it = soundMap.find(id);
	if (it == soundMap.end()) {
		logger.error("AudioSystem::playSFX: Failed to find audio resource with ID: " + std::to_string(id));
		return 0;
	}

	logger.log("AudioSystem::playSFX: Playing SFX with audio resource ID: " + std::to_string(id));

	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = fmodSystem->playSound(it->second, sfxChannelGroup, false, &channel);
	if (result != FMOD_OK) {
		logger.error("AudioSystem::playSFX: Failed to play SFX sound for audio resource ID: " + std::to_string(id));
		return 0;
	}

	AudioInstance inst{};
	inst.channel = channel;
	inst.isPlaying = true;
	inst.volume = sfxVolume;
	++totalInstanceCount;
	audioInstances[totalInstanceCount] = std::move(inst);

	channel->setVolume(sfxVolume);
	return totalInstanceCount;
}
