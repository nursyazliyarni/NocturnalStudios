/*!------------------------------------------------------------------------------------------------
    File: AudioManager.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the AudioManager script class which is responsible 
        for managing audio playback within the engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the AudioManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Scripts/AudioManager.h"
#include "Audio/AudioEngine.h"

namespace NocturnalEngine
{
    /*!*************************************************************************
    Default constructor for Audio Manager
    ****************************************************************************/
    AudioManager::AudioManager() : BGMvol { 20.f }, SFXvol { 50.f }, Mastervol { 50.f } {}
    /*!*************************************************************************
    Returns a new copy of AudioManager Script
    ****************************************************************************/
	AudioManager* AudioManager::Clone() const
	{
		return new AudioManager(*this);
	}
    /*!*************************************************************************
    Start State of AudioManager Script
    ****************************************************************************/
	void AudioManager::Start()
	{
        if (p_ecs.HaveComponent<Audio>(GetScriptEntityID())) 
        {
            auto& audio = p_ecs.GetComponent<Audio>(GetScriptEntityID());
            for (int i = 0; i < audio.GetSize(); i++) {
                p_Audio->SetLooping(audio[i].mAudioPath, audio[i].is_Looping);
            }
        }
	}
    /*!*************************************************************************
    Update Loop of AudioManager Script
    ****************************************************************************/
	void AudioManager::Update(float Frametime)
	{
        UNREFERENCED_PARAMETER(Frametime);
        p_Audio->Update();
        //update volume values
        //check sounds and update
        if (p_ecs.HaveComponent<Audio>(GetScriptEntityID())) {
            auto& audio = p_ecs.GetComponent<Audio>(GetScriptEntityID());
            for (int i = 0; i < audio.GetSize(); i++) {
                //check if sound is playing
                if (audio[i].triggered)
                {
                    audio[i].is_Playing = p_Audio->IsPlaying(audio[i].mChannel);
                    if (audio[i].is_Playing == false) {
                        audio[i].triggered = false;
                    }
                }
                if (audio[i].is_Looping == false && audio[i].should_play == true && audio[i].is_Playing == false) {
                    //play sound
                    audio[i].mChannel = p_Audio->PlaySound("Assets/Audio/" + audio[i].mAudioPath, audio[i].mChannelGroup);
                    audio[i].should_play = false;
                    audio[i].triggered = true;
                }
                if (audio[i].is_Looping == true && audio[i].is_Playing == false) {
                    // For BGM, check if it's already playing (persisted from previous scene)
                    std::string fullPath = "Assets/Audio/" + audio[i].mAudioPath;
                    if (audio[i].mChannelGroup == Audio::AudioType::BGM && p_Audio->IsBGMPathPlaying(fullPath))
                    {
                        // BGM is already playing from previous scene, just update our tracking
                        audio[i].is_Playing = true;
                        audio[i].triggered = true;
                        // Note: mChannel may not match the actual channel, but is_Playing check will still work
                    }
                    else
                    {
                        //play sound
                        audio[i].mChannel = p_Audio->PlaySound(fullPath, audio[i].mChannelGroup);
                        audio[i].triggered = true;
                    }
                }
                if (audio[i].should_stop == true) {
                    p_Audio->StopChannel(audio[i].mChannel);
                    audio[i].is_Playing = false;
                    audio[i].triggered = false;
                    audio[i].should_stop = false;
                }
                audio[i].should_play = false;
            }
        }
	}
    /*!*************************************************************************
    End State for Audio Manager
    ****************************************************************************/
    void AudioManager::End()
    {
        
    }

    std::string AudioManager::GetScriptName()
    {
        return "AudioManager";
    }
}