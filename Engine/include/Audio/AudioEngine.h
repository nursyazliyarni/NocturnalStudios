/*!------------------------------------------------------------------------------------------------
    File: AudioEngine.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration of AudioEngine class.

    Primary Author:
         LAU XIANG YANG (100%)
		- Created the audio engine class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "pch.h"
#include "fmod.hpp"
#include "fmod_dsp_effects.h"
#include "ECS/Components/Audio.h"

#define p_Audio CAudioEngine::GetInstance()

class CAudioEngine
{

    FMOD::System* mpSystem;

    int mNextChannelId = 1;

    std::map<std::string, FMOD::Sound*> mSoundMap;


    FMOD::ChannelGroup* BGM, * SFX, * Master;
    FMOD::Sound* Msound;
    
    // Track currently playing BGM path to prevent restart on scene transitions
    std::string mCurrentBGMPath;
    int mCurrentBGMChannelId = -1;

    // DSP effect units applied on the Master channel group
    FMOD::DSP* mDspLowpass{ nullptr };
    FMOD::DSP* mDspHighpass{ nullptr };
    FMOD::DSP* mDspEcho{ nullptr };
    FMOD::DSP* mDspReverb{ nullptr };

    AudioEffectSettings mCurrentEffects;
public:
    std::map<int, FMOD::Channel*> mChannelMap;

    enum class channel_groups { master, bgm, sfx };
    static std::unique_ptr<CAudioEngine>& GetInstance();

    void Init();
    void Update();
    void Release();
    void ErrorCheck(FMOD_RESULT result);

    FMOD::Sound* Loadsound(const std::string& strSoundName, bool bLooping = false);

    int PlaySound(const std::string& strSoundName, NocturnalEngine::Audio::AudioType chgrp);
    void PauseSound(int channelID);
    void UnpauseSound(int channelID);
    void StopChannel(channel_groups chan);
    void SetVolume(int channelID, float vol);
    void StopChannel(int channelID);
    float GetVolume(int channelID);
    void SetLooping(const std::string& strSoundName, bool in);
    bool IsPlaying(int nChannelId) const;
    float dbToVolume(float db);
    float VolumeTodB(float volume);
    int GetPlayingChannels() { return (int)mChannelMap.size(); }
    void SetVolumeByChannel(FMOD::ChannelGroup* ch, float volume) { ch->setVolume(volume); }
    float GetVolumeByChannel(FMOD::ChannelGroup* ch);
    FMOD::ChannelGroup* GetMasterChannelGroup() { return Master; }
    FMOD::ChannelGroup* GetBGMChannelGroup() { return BGM; }
    FMOD::ChannelGroup* GetSFXChannelGroup() { return SFX; }
    
    // BGM persistence across scene transitions
    bool IsBGMPathPlaying(const std::string& strSoundPath) const;
    const std::string& GetCurrentBGMPath() const { return mCurrentBGMPath; }
    void ClearBGMTracking();

    // Audio effects (DSP) management
    void ApplyEffects(const AudioEffectSettings& settings);
    const AudioEffectSettings& GetCurrentEffects() const { return mCurrentEffects; }

    // Audio fade-out for smooth transitions
    static constexpr float DEFAULT_FADE_DURATION = 0.5f;
    void FadeOutChannel(int channelID, float fadeDurationSeconds = DEFAULT_FADE_DURATION);
    void FadeOutAndStopChannel(int channelID, float fadeDurationSeconds = DEFAULT_FADE_DURATION);
    void FadeOutChannelGroup(channel_groups chan, float fadeDurationSeconds = DEFAULT_FADE_DURATION);

private:
    // Channels that are fading out and should be stopped once the fade completes
    struct FadeOutInfo
    {
        int channelId;
        unsigned long long stopAtDspClock;
    };
    std::vector<FadeOutInfo> mPendingFadeStops;
    void ProcessPendingFadeStops();
    unsigned long long GetSampleRateFromSystem() const;
};

#endif