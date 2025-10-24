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

class AudioSystem : public System {
public:
	struct AudioInstance {
		AudioInstanceID instanceID = 0;
		FMOD::Channel* channel = nullptr;
		bool isPlaying = false;
		float volume = 1.0f;

		bool persistAfterEntityDestruction = false;		// if false, the audio instance will be deleted when the entity is destroyed
		bool persistOnSceneChange = false;				// if false, the audio instance will be deleted during scene change
		EntityID parentEntityID = INVALID_ENTITY_ID;
		bool toDestroy = false;							// for delayed deletion when the audio finishes playing
	};

	AudioSystem(std::string name, ECS& ecs, ResourceManager& rm);
	~AudioSystem();

	void update(double deltaTime) final;
	std::optional<AudioInstanceID> createAudioInstance(AudioData::IndividualAudioData const& audioData, EntityID id);

	void restartSound(AudioInstanceID instanceID);
	bool resumeSound(AudioInstanceID instanceID);
	void pauseSound(AudioInstanceID instanceID);
	void modifyVolume(AudioInstanceID instanceID, float volume);
	float getVolume(AudioInstanceID instanceID);

	void pauseAllSFX();
	void resumeAllSFX();

	FMOD::System* getSystem() const;

	void destroyAudioInstance(AudioInstanceID instanceID);
	void handleEntityDestruction(EntityID entityID);
	void handleCompletedAudioInstance(FMOD::Channel* channel);
	void deleteAllAudioInstances(bool forceDeletePersistentSounds = true);

	bool registerNewAudioResource(AudioData& audioData, EntityID entityID);
	void loadSound(Audio const& audio);
	void toggleLowPassFilter(bool enable);

	void setMasterVolume(float volume);
	void setSFXVolume(float volume);
	void setBGMVolume(float volume);
	void setReserved1Volume(float volume);
	void setReserved2Volume(float volume);

	float getMasterVolume() const;
	float getSFXVolume() const;
	float getBGMVolume() const;
	float getReserved1Volume() const;
	float getReserved2Volume() const;

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
