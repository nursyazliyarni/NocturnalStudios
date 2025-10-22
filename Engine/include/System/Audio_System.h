
#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <../fmod/fmod.hpp>
#include <../fmod/fmod_errors.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>

#include "../include/math/Vector.h"

namespace Audio {
	class AudioSystem {
	public:
		AudioSystem();
		~AudioSystem();

		void update();
		FMOD_RESULT result;
		FMOD::System* system;
		unsigned int version;
		void* extradriverdata = 0;
		int nextChannelID = 0;

		typedef std::map<std::string, FMOD::Sound*> SoundMap;
		typedef std::map<int, FMOD::Channel*> ChannelMap;

		SoundMap soundMap;
		ChannelMap channelMap;
	};

	class AudioEngine {
	public:
		static void Init();
		static void Update();
		static void Shutdown();
		static int ErrorCheck(FMOD_RESULT result);

		void LoadSound(const std::string& soundName, bool is3d = true, bool looping = false, bool stream = false);
		void UnloadSound(const std::string& soundName);
		int PlaySound(const std::string& soundName, const Vector2D& position = Vector2D(0.0f, 0.0f), float volume = 0.0f);

		void StopChannel(int channelID);
		void StopAllChannels();
		void SetChannelVolume(int channelID, float volume);

		bool IsPlaying(int channelID);

		float dbToVolume(float db);
		float VolumeTodb(float volume);

		FMOD_VECTOR VectorToFmod(const Vector2D& position);
	};
}

#endif // AUDIO_SYSTEM_H