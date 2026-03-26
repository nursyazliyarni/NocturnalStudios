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
#include "ECS/Components/Transform.h"
#include "ECS/Components/Tag.h"
#include <algorithm>
#include <cmath>
#include <cctype>

namespace NocturnalEngine
{
    namespace
    {
        constexpr float SPATIAL_MAX_DISTANCE = 20.0f;
    }

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
            // Apply audio effects from the component settings
            p_Audio->ApplyEffects(audio.GetEffectSettings());
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

            vec2D sourcePos{};
            if (p_ecs.HaveComponent<Transform>(GetScriptEntityID()))
            {
                sourcePos = p_ecs.GetComponent<Transform>(GetScriptEntityID()).GetPos();
            }

            vec2D listenerPos{};
            bool listenerFound = false;
            for (const Entity e : p_ecs.GetAliveEntitiesSet())
            {
                if (!p_ecs.HaveComponent<Transform>(e) || !p_ecs.HaveComponent<Tag>(e))
                    continue;

                std::string tag = p_ecs.GetComponent<Tag>(e).GetTag();
                std::transform(tag.begin(), tag.end(), tag.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });

                if (tag.find("player") != std::string::npos)
                {
                    listenerPos = p_ecs.GetComponent<Transform>(e).GetPos();
                    listenerFound = true;
                    break;
                }
            }

            if (!listenerFound)
            {
                listenerPos = vec2D(0.0f, 0.0f);
            }

            const float dx = sourcePos.x - listenerPos.x;
            const float dy = sourcePos.y - listenerPos.y;
            const float distance = std::sqrt((dx * dx) + (dy * dy));
            const float attenuation = std::clamp(1.0f - (distance / SPATIAL_MAX_DISTANCE), 0.0f, 1.0f);
            const float pan = std::clamp(dx / SPATIAL_MAX_DISTANCE, -1.0f, 1.0f);

            // Apply audio effects every frame so editor changes are reflected in real-time
            p_Audio->ApplyEffects(audio.GetEffectSettings());
            for (int i = 0; i < audio.GetSize(); i++) {
                // check if sound is playing
                if (audio[i].triggered) {
                    audio[i].is_Playing = p_Audio->IsPlaying(audio[i].mChannel);
                    if (audio[i].is_Playing == false) {
                        audio[i].triggered = false;
                    }
                }
                // play SFX
                if (audio[i].is_Looping == false && audio[i].should_play == true && audio[i].is_Playing == false) {
                    audio[i].mChannel = p_Audio->PlaySound("Assets/Audio/" + audio[i].mAudioPath, audio[i].mChannelGroup);
                    const bool applySpatial = audio[i].mChannelGroup != Audio::AudioType::BGM;
                    p_Audio->SetVolume(audio[i].mChannel, audio[i].volumeLimit * (applySpatial ? attenuation : 1.0f));
                    p_Audio->SetPan(audio[i].mChannel, applySpatial ? pan : 0.0f);
                    audio[i].should_play = false;
                    audio[i].triggered = true;
                }
                // play BGM
                if (audio[i].is_Looping == true && audio[i].is_Playing == false) {
                    std::string fullPath = "Assets/Audio/" + audio[i].mAudioPath;
                    if (audio[i].mChannelGroup == Audio::AudioType::BGM && p_Audio->IsBGMPathPlaying(fullPath)) {
                        audio[i].is_Playing = true;
                        audio[i].triggered = true;
                    }
                    else {
                        audio[i].mChannel = p_Audio->PlaySound(fullPath, audio[i].mChannelGroup);
                        p_Audio->SetVolume(audio[i].mChannel, audio[i].volumeLimit);
                        p_Audio->SetPan(audio[i].mChannel, 0.0f);
                        audio[i].triggered = true;
                    }
                }

                if (audio[i].triggered)
                {
                    const bool applySpatial = audio[i].mChannelGroup != Audio::AudioType::BGM;
                    p_Audio->SetVolume(audio[i].mChannel, audio[i].volumeLimit * (applySpatial ? attenuation : 1.0f));
                    p_Audio->SetPan(audio[i].mChannel, applySpatial ? pan : 0.0f);
                }

                // stop
                if (audio[i].should_stop == true) {
                    p_Audio->FadeOutAndStopChannel(audio[i].mChannel);
                    audio[i].is_Playing = false;
                    audio[i].triggered = false;
                    audio[i].should_stop = false;
                }
                
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