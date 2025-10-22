#pragma once

#include <string>
#include <unordered_map>

#include <../fmod/fmod.hpp>
#include <../fmod/fmod_errors.h>

#include "../core/Logger.h"
extern Logger logger;

class AudioSystem {
public:
	AudioSystem() = default;
	~AudioSystem();

	bool init();
	void update();
	void shutdown();

	bool loadSound(const std::string& key, const std::string& path, bool stream = false, bool loop = false);
	bool playBGM(const std::string& key, float volume = 1.0f);
	void stopBGM();
	bool playSFX(const std::string& key, float volume = 1.0f);

	void setBGMVolume(float volume);
	void setSFXVolume(float volume);

private:
	FMOD_SYSTEM* fmodSystem{ nullptr };
	std::unordered_map<std::string, FMOD_SOUND*> soundMap;
	FMOD_CHANNEL* bgmChannel{ nullptr };

	float bgmVolume{ 1.0f };
	float sfxVolume{ 1.0f };

	void logFMODError(FMOD_RESULT result, const std::string& context);
};
