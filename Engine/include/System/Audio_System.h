#pragma once

#include <string>
#include <numeric>
#include <optional>
#include <../fmod/fmod.hpp>

namespace FMOD {
	class System;
	class Sound;
	class Channel;
}

#include "../ECS/ECS_Def.h"
#include "../ECS/ECS_Component.h"
#include "../resources/Resource.h"
#include "System.h"

using AudioID = std::size_t;
using EntityID = std::size_t;

class ResourceManager;
class ECS;
class Audio;
struct Individual;

class AudioSystem : public System {
public:
	struct AudioInstance {
		AudioID id;
		FMOD::Channel* channel = nullptr;
		bool isPlaying = false;
		bool locality = false;
		float volume = 1.0f;
		float distanceMult = 1.0f;

		bool persistOnEntityDestruction = false;
		bool persistOnSceneChange = false;
		EntityID parentEntity = INVALID_ENTITY_ID;

		Vector2D lastPosition = {};
		bool toDestroy = false;
	};

	struct AttenuationParams {
		float volumeMult = 1.0f;
		float panMult = 0.0f;
	};

	AudioSystem(std::string anem, ECS& ecs, ResourceManager& resourceMgr);
	~AudioSystem();

	void update(double deltaTime) final;
	std::optional<AudioID> createSoundInstance(AudioData::Individual const& audioData, EntityID entityID);

	void restartSound(AudioID audioID);
	void resumeSound(AudioID audioID);
	void pauseSound(AudioID audioID);
	void modifySoundVolume(AudioID audioID, float newVolume);
	float getSoundVolume(AudioID audioID) const;

	void pauseAllSFX();
	void resumeAllSFX();

	void handleSystemResume();
	void handleSystemPause();

	FMOD::System* getSystem() const;

	void destroySoundInstance(AudioID audioID);
	void handleEntityDestruction(EntityID entityID);
	void deleteAllSoundInstances(bool deletePersistentSounds = true);
	void handleFinishedSoundInstances(FMOD::Channel* channel);

	bool initWithNewScene();
	bool registerNewSound(AudioData& audioData, EntityID entityID);

	void toggleLowPassFilter(bool enable);

	void setMasterVolume(float volume);
	void setMusicVolume(float volume);
	void setSFXVolume(float volume);

	float getMasterVolume() const;
	float getMusicVolume() const;
	float getSFXVolume() const;

private:
	AttenuationParams calculateAttenuationParams(const Vector2D& position, float distanceMult);

	float bgmVolume;
	float sfxVolume;

	ResourceManager& resourceManager;
	FMOD::System* fmodSystem;

	std::unordered_map<ResourceID, FMOD::Sound*> sounds;
	std::unordered_map<AudioID, AudioInstance> audioInstances;

	AudioID totalAudioInstances = 0;
	EntityID listenerEntity = INVALID_ENTITY_ID;
	bool resetGame = false;

	FMOD::ChannelGroup* masterGroup = nullptr;
	FMOD::ChannelGroup* musicGroup = nullptr;
	FMOD::ChannelGroup* SFXGroup = nullptr;
	FMOD::ChannelGroup* Reserve1Group = nullptr;
	FMOD::ChannelGroup* Reserve2Group = nullptr;
	FMOD::ChannelGroup* Reserve3Group = nullptr;

	FMOD::DSP* lowPassFilter = nullptr;
};
