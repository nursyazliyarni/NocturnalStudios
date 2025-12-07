/*!------------------------------------------------------------------------------------------------
	File: Audio_System.h
	Project: NocturnalStudio

	This file contains the declaration of the AudioSystem class, which manages the integration
	of the FMOD library into the customize game engine. It handles the initialization,
	frame updates, and rendering of audio components, as well as providing various UI elements
	for debugging and interaction within the engine.

	Primary Author:
		Lau Xiang Yang (100%)
		- Created the AudioSystem class and its specializations.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <numeric>
#include <optional>
#include <unordered_map>
#include <iostream>
#include <algorithm>

//#include <../fmod/fmod.hpp>
//#include <../fmod/fmod_errors.h>

#include "fmod.hpp"
#include "fmod_errors.h"

#include "ECS/ECS_Component.h"
#include "ECS/ECS_Def.h"
#include "ECS/ECS.h"
#include "resources/Resource.h"
#include "resources/ResourceManager.h"
#include "System.h"
#include "../core/Logger.h"
extern Logger logger;

using AudioInstanceID = std::size_t;
using EntityID = std::size_t;

class ResourceManager;
class ECS;
class Audio;
struct IndividualAudioData;

/**
 * @brief System responsible for audio playback and FMOD integration.
 *
 * Manages loaded FMOD::Sound objects, active playback channels (AudioInstance),
 * channel groups for volume control (Master/BGM/SFX/Reserved), and a low-pass DSP.
 */
class AudioSystem : public System {
public:
    /**
     * @brief Runtime representation of a playing (or paused) audio channel.
     *
     * Fields track the FMOD channel pointer, playback state, volume and
     * persistence flags for entity/scene lifetimes. channelResourceID stores
     * the ResourceID used to create this channel (0 for default sound).
     */
    struct AudioInstance {
        AudioInstanceID instanceID = 0;                     /**< Unique instance ID */
        FMOD::Channel* channel = nullptr;                   /**< FMOD channel pointer */
        bool isPlaying = false;                             /**< True when playing */
        float volume = 1.0f;                                /**< Per-instance volume [0..1] */
        bool persistAfterEntityDestruction = false;         /**< Don't stop when parent entity is destroyed */
        bool persistOnSceneChange = false;                  /**< Don't stop automatically on scene change */
        EntityID parentEntityID = INVALID_ENTITY_ID;       /**< Owning entity ID */
        bool toDestroy = false;                             /**< Marked for deletion on next update */
        ResourceID channelResourceID = 0;                   /**< ResourceID used to create the channel */
    };

    /**
     * @brief Construct an AudioSystem.
     * @param name Human readable name for the system (forwarded to base System).
     * @param ecs Reference to the engine ECS instance.
     * @param rm Reference to the ResourceManager used to resolve audio resources.
     *
     * Initializes FMOD, channel groups and the low-pass DSP, and preloads audio assets
     * from the ResourceManager.
     */
    AudioSystem(std::string name, ECS& ecs, ResourceManager& rm);

    /**
     * @brief Destroy the AudioSystem and release FMOD resources.
     *
     * Stops playback, releases loaded sounds, DSPs, channel groups and the FMOD system.
     */
    ~AudioSystem();

    /**
     * @brief Per-frame update; must be called once per frame.
     * @param deltaTime Frame delta time in seconds (unused currently).
     *
     * Calls fmodSystem->update(), applies per-instance volume settings and
     * removes instances flagged for destruction.
     */
    void update(double deltaTime) final;

    /**
     * @brief Create and start an audio instance from audio data.
     * @param audioData Per-instance playback configuration (resource id, volume, loop, group, persistence).
     * @param id Parent entity id to associate with this instance (can be INVALID_ENTITY_ID).
     * @return AudioInstanceID if successful, std::nullopt on error.
     */
    std::optional<AudioInstanceID> createAudioInstance(AudioData::IndividualAudioData const& audioData, EntityID id);

    /**
     * @brief Reset an instance's play head to the start (0 ms).
     * @param instanceID ID returned by createAudioInstance or playBGM/playSFX.
     */
    void restartSound(AudioInstanceID instanceID);

    /**
     * @brief Resume a paused audio instance.
     * @param instanceID Instance ID to resume.
     * @return true if resumed successfully, false if instance invalid or underlying channel failed.
     */
    bool resumeSound(AudioInstanceID instanceID);

    /**
     * @brief Pause a playing audio instance.
     * @param instanceID ID of the instance to pause.
     */
    void pauseSound(AudioInstanceID instanceID);

    /**
     * @brief Change the stored volume of an instance and apply it to the FMOD channel.
     * @param instanceID ID of the instance to modify.
     * @param volume New volume in range [0.0f, 1.0f].
     */
    void modifyVolume(AudioInstanceID instanceID, float volume);

    /**
     * @brief Get the stored per-instance volume.
     * @param instanceID Instance ID to query.
     * @return Stored volume in [0..1]; returns 0.0f and logs on invalid ID.
     */
    float getVolume(AudioInstanceID instanceID);

    /**
     * @brief Pause all SFX routed through the SFX channel group.
     */
    void pauseAllSFX();

    /**
     * @brief Pause all BGM routed through the BGM channel group.
     */
    void pauseAllBGM();

    /**
     * @brief Resume all SFX routed through the SFX channel group.
     */
    void resumeAllSFX();

    /**
     * @brief Resume all BGM routed through the BGM channel group.
     */
    void resumeAllBGM();

    /**
     * @brief Return the raw FMOD::System pointer.
     * @return FMOD::System* or nullptr if not initialized. Use carefully.
     */
    FMOD::System* getSystem() const;

    /**
     * @brief Immediately stop and remove a single audio instance.
     * @param instanceID ID of the instance to destroy.
     */
    void destroyAudioInstance(AudioInstanceID instanceID);

    /**
     * @brief Handle cleanup for an entity that is being destroyed.
     * @param entityID ID of the entity being destroyed.
     *
     * Either clears parent ownership for persistent sounds or stops and removes non-persistent ones.
     */
    void handleEntityDestruction(EntityID entityID);

    /**
     * @brief Mark an audio instance for deletion when FMOD reports playback end.
     * @param channel FMOD channel pointer that finished playback.
     *
     * Intended to be called from the FMOD channel callback to schedule safe deletion on the next update.
     */
    void handleCompletedAudioInstance(FMOD::Channel* channel);

    /**
     * @brief Remove all audio instances.
     * @param forceDeletePersistentSounds If true, deletes even sounds marked persistOnSceneChange.
     */
    void deleteAllAudioInstances(bool forceDeletePersistentSounds = true);

    /**
     * @brief Called when a new AudioData component is registered for an entity.
     * @param audioData Component containing audios to register/optionally start.
     * @param entityID Entity to associate with any started instances.
     * @return true on success, false if any requested audio failed to start.
     */
    bool registerNewAudioResource(AudioData& audioData, EntityID entityID);

    /**
     * @brief Load an Audio resource into an FMOD::Sound and register it internally.
     * @param audio Audio resource to load (provides file path and resource id).
     *
     * On failure logs and, on certain errors, shows a popup on Windows.
     */
    void loadSound(Audio const& audio);

    /**
     * @brief Enable or disable the low-pass DSP applied to BGM.
     * @param enable true to enable the low-pass, false to bypass it.
     */
    void toggleLowPassFilter(bool enable);

    /**
     * @brief Set master channel group volume [0..1].
     * @param volume New master volume.
     */
    void setMasterVolume(float volume);

    /**
     * @brief Set SFX channel group volume [0..1].
     * @param volume New SFX volume.
     */
    void setSFXVolume(float volume);

    /**
     * @brief Set BGM channel group volume [0..1].
     * @param volume New BGM volume.
     */
    void setBGMVolume(float volume);

    /**
     * @brief Set Reserved1 channel group volume [0..1].
     * @param volume New Reserved1 volume.
     */
    void setReserved1Volume(float volume);

    /**
     * @brief Set Reserved2 channel group volume [0..1].
     * @param volume New Reserved2 volume.
     */
    void setReserved2Volume(float volume);

    /**
     * @brief Query current master volume.
     * @return Master volume in [0..1].
     */
    float getMasterVolume() const;

    /**
     * @brief Query current SFX volume.
     * @return SFX volume in [0..1].
     */
    float getSFXVolume() const;

    /**
     * @brief Query current BGM volume.
     * @return BGM volume in [0..1].
     */
    float getBGMVolume() const;

    /**
     * @brief Query current Reserved1 volume.
     * @return Reserved1 group volume in [0..1].
     */
    float getReserved1Volume() const;

    /**
     * @brief Query current Reserved2 volume.
     * @return Reserved2 group volume in [0..1].
     */
    float getReserved2Volume() const;

    /**
     * @brief Play a preloaded audio resource as background music (BGM).
     * @param id ResourceID to play.
     * @return AudioInstanceID of the created instance on success, or 0 on failure.
     */
    AudioInstanceID playBGM(ResourceID id);

    /**
     * @brief Play a preloaded audio resource as a sound effect (SFX).
     * @param id ResourceID to play.
     * @return AudioInstanceID of the created instance on success, or 0 on failure.
     */
    AudioInstanceID playSFX(ResourceID id);

private:
    float bgmVolume;
    float sfxVolume;

    ResourceManager& resourceManager;

    std::unordered_map<AudioInstanceID, AudioInstance> audioInstances;		// stores each instance of playing audio
    std::unordered_map<ResourceID, FMOD::Sound*> soundMap;					// stores actual audio assets

    AudioInstanceID totalInstanceCount = 0;
    
    FMOD::System* fmodSystem;
    FMOD::ChannelGroup* masterChannelGroup = nullptr;
    FMOD::ChannelGroup* bgmChannelGroup = nullptr;
    FMOD::ChannelGroup* sfxChannelGroup = nullptr;
    FMOD::ChannelGroup* reserved1ChannelGroup = nullptr;
    FMOD::ChannelGroup* reserved2ChannelGroup = nullptr	;
    FMOD::DSP* lowPassFilter = nullptr;
};

// global audio system pointer
extern AudioSystem* gAudioSystem;
