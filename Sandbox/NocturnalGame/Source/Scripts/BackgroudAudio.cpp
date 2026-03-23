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

namespace NocturnalEngine {
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
			if (a.GetArr().size()>bgAudioIndex)
			{
				a[bgAudioIndex] .is_Looping = true;
			}

		}
	}

	void BackgroundAudio::Update(float Frametime)
	{
		UNREFERENCED_PARAMETER(Frametime);
		if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()) && (p_ecs.GetComponent<Audio>(GetScriptEntityID()).GetSize() > 0))
		{
			Audio& a{ p_ecs.GetComponent<Audio>(GetScriptEntityID()) };
			if (a.GetArr().size() > bgAudioIndex)
			{
				a[bgAudioIndex].is_Looping = true;
			}
		}
	}

	void BackgroundAudio::End()
	{
		if (p_ecs.HaveComponent<Audio>(GetScriptEntityID()))
		{
			Audio& a{ p_ecs.GetComponent<Audio>(GetScriptEntityID()) };
			if (a.GetArr().size() > bgAudioIndex)
			{
				a[bgAudioIndex].should_stop = true;
				a[bgAudioIndex].is_Looping = false;
			}
		}
		
	}

	std::string BackgroundAudio::GetScriptName()
	{
		return "BackgroundAudio";
	}

}