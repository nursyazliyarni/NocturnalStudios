/*!------------------------------------------------------------------------------------------------
    File: Audio.h
    Project: NocturnalStudio

	Brief:
		Contains the component related to audio in the ECS
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Audio.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#pragma warning( disable : 26495 )

struct AudioEffectSettings
{
	bool  lowpassEnabled{ false };
	float lowpassCutoff{ 5000.0f };
	float lowpassResonance{ 1.0f };

	bool  highpassEnabled{ false };
	float highpassCutoff{ 10.0f };
	float highpassResonance{ 1.0f };

	bool  echoEnabled{ false };
	float echoDelay{ 500.0f };
	float echoFeedback{ 50.0f };
	float echoDryLevel{ 0.0f };
	float echoWetLevel{ -6.0f };

	bool  reverbEnabled{ false };
	float reverbDecayTime{ 1500.0f };
	float reverbEarlyDelay{ 7.0f };
	float reverbLateDelay{ 11.0f };
	float reverbHFReference{ 5000.0f };
	float reverbDiffusion{ 100.0f };
	float reverbDensity{ 100.0f };
	float reverbWetLevel{ -6.0f };
	float reverbDryLevel{ 0.0f };
};

namespace NocturnalEngine
{

	class Audio : public IComponent
	{
	public:
		enum class AudioType
		{
			NONE,
			MASTER,
			BGM,
			SFX
		};
		struct AudioPiece {
			std::string mAudioPath{};
			AudioType mChannelGroup{};
			int mChannel{};
			bool is_Looping{};
			bool should_play{};
			bool should_stop{};
			bool is_Playing{};
			bool triggered{};
		};
		Audio();
		~Audio() = default;
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
		bool ToEditor()override;
		
		std::vector<AudioPiece>& GetArr() { return AudioArr; }
		void SetArr(std::vector<AudioPiece> in) { AudioArr = in; }
		int GetSize() { return (int)AudioArr.size(); }
		AudioPiece& operator[](int i) { return AudioArr[i]; }

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

		float& GetBGMVolume() { return bgmVol; }
		float& GetSFXVolume() { return sfxVol; }
		float& GetMasterVolume() { return masterVol; }

		void SetBGMVolume(float vol) { bgmVol = vol; }
		void SetSFXVolume(float vol) { sfxVol = vol; }
		void SetMasterVolume(float vol) { masterVol = vol; }

		bool IsRandomized()const { return randomFromArray; }

		AudioEffectSettings& GetEffectSettings() { return effectSettings; }
		const AudioEffectSettings& GetEffectSettings() const { return effectSettings; }
		void SetEffectSettings(const AudioEffectSettings& s) { effectSettings = s; }
	private:
		std::vector<AudioPiece> AudioArr;
		float bgmVol;
		float sfxVol;
		float masterVol;
		bool randomFromArray{ false };
		AudioEffectSettings effectSettings;

		std::vector < std::pair<std::string, std::filesystem::directory_entry>> LoadAudioFromFiles();
	};
}
