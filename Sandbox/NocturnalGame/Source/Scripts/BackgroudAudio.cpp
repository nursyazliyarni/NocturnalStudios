/*!------------------------------------------------------------------------------------------------
    File: BackgroundAudio.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the BackgroundAudio script class which is responsible 
        for managing background audio playback within the engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the BackgroundAudio class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/BackgroundAudio.h"
#include "Audio/AudioEngine.h"
#include "ECS/Components/Audio.h"
#include "ECS/SceneManager.h"
#include "Scripts/GlobalGameManager.h"

namespace NocturnalEngine {
	namespace
	{
		bool IsMenuFlowScene(const std::string& scenePath)
		{
			return scenePath == "Assets/Scene/Menu.json" ||
				scenePath == "Assets/Scene/Option.json" ||
				scenePath == "Assets/Scene/HowToPlay.json" ||
				scenePath == "Assets/Scene/Quit.json";
		}

		void ConfigureLevelBGM(Audio& audio, int bgAudioIndex)
		{
			std::vector<int> bgmTrackIndices{};
			bgmTrackIndices.reserve(static_cast<size_t>(audio.GetSize()));

			for (int i = 0; i < audio.GetSize(); ++i)
			{
				if (audio[i].mChannelGroup == Audio::AudioType::BGM)
				{
					bgmTrackIndices.push_back(i);
				}
			}

			if (bgmTrackIndices.empty())
			{
				if (bgAudioIndex >= 0 && bgAudioIndex < audio.GetSize())
				{
					audio[bgAudioIndex].is_Looping = true;
				}
				return;
			}

			const int levelOffset = static_cast<int>(p_GameManager.CurrentLevel() > 0 ? p_GameManager.CurrentLevel() - 1 : 0);
			int selectedBgmSlot = bgAudioIndex + levelOffset;
			if (selectedBgmSlot < 0)
			{
				selectedBgmSlot = 0;
			}
			if (selectedBgmSlot >= static_cast<int>(bgmTrackIndices.size()))
			{
				selectedBgmSlot = static_cast<int>(bgmTrackIndices.size()) - 1;
			}

			const int selectedTrackIndex = bgmTrackIndices[static_cast<size_t>(selectedBgmSlot)];

			for (const int idx : bgmTrackIndices)
			{
				Audio::AudioPiece& track = audio[idx];
				const bool isSelectedTrack = (idx == selectedTrackIndex);

				if (isSelectedTrack)
				{
					track.is_Looping = true;
					track.should_stop = false;
				}
				else
				{
					track.is_Looping = false;
					track.should_play = false;
					if (track.is_Playing || track.triggered)
					{
						track.should_stop = true;
					}
				}
			}
		}
	}

	BackgroundAudio::BackgroundAudio(){}

	BackgroundAudio* BackgroundAudio::Clone() const
	{
		return new BackgroundAudio(*this);
	}

	void BackgroundAudio::Start()
	{
		//check if it has an audio loaded 
		if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && (p_ecs.GetComponent<Audio>(GetScriptEntityID()).GetSize() > 0) )
		{
			Audio& a{ p_ecs.GetComponent<Audio>(GetScriptEntityID()) };
			ConfigureLevelBGM(a, bgAudioIndex);

		}
	}

	void BackgroundAudio::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);
		if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && (p_ecs.GetComponent<Audio>(GetScriptEntityID()).GetSize() > 0))
		{
			Audio& a{ p_ecs.GetComponent<Audio>(GetScriptEntityID()) };
			ConfigureLevelBGM(a, bgAudioIndex);
		}
	}

	void BackgroundAudio::End()
	{
		if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()))
		{
			const std::string& nextScene = p_Scene->GetQueuedSceneToLoad();
			const bool preserveMenuBgm = IsMenuFlowScene(nextScene);
			Audio& a{ p_ecs.GetComponent<Audio>(GetScriptEntityID()) };
			for (int i = 0; i < a.GetSize(); ++i)
			{
				if (a[i].mChannelGroup != Audio::AudioType::BGM)
				{
					continue;
				}

				if (!preserveMenuBgm && (a[i].triggered || a[i].is_Playing))
				{
					p_Audio->FadeOutAndStopChannel(a[i].mChannel);
				}

				if (preserveMenuBgm)
				{
					a[i].should_stop = false;
					a[i].should_play = false;
					continue;
				}

				a[i].should_stop = true;
				a[i].should_play = false;
				a[i].is_Playing = false;
				a[i].triggered = false;
				a[i].is_Looping = false;
			}
		}
		
	}

	std::string BackgroundAudio::GetScriptName()
	{
		return "BackgroundAudio";
	}

}
