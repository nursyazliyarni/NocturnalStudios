/*!------------------------------------------------------------------------------------------------
    File: AudioEngine.cpp
    Project: NocturnalStudio

	Brief:
		This file utilises FMOD API calls to load, play, pause, stop and set volume.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the audio engine class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Audio/AudioEngine.h"
#include "ResourceManager/ResourceManager.h"
#include <algorithm>

std::unique_ptr<CAudioEngine> m_Instance;

std::unique_ptr<CAudioEngine>& CAudioEngine::GetInstance()
{
    if (!m_Instance)
    {
        m_Instance = std::make_unique<CAudioEngine>();
    }
    return m_Instance;
}

/*!*************************************************************************
Error check for FMOD audio
****************************************************************************/
void CAudioEngine::ErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK) {
    }
}

/*!*************************************************************************
Load sound function stores the audio selected into a map and calls the FMOD
API create sound
****************************************************************************/
FMOD::Sound* CAudioEngine::Loadsound(const std::string& strSoundName, bool b_Looping)
{
    auto tFoundIt = mSoundMap.find(strSoundName); //find if sound is loaded
    FMOD::Sound* pSound = nullptr; //create sound
    if (tFoundIt != mSoundMap.end())
        return pSound; //if loaded exit function

    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b_Looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

    CAudioEngine::ErrorCheck(mpSystem->createSound(strSoundName.c_str(), eMode, NULL, &pSound));
    if (pSound)
    {
        mSoundMap[strSoundName] = pSound; //if sound is created store in sound map
    }
    return pSound;
}

/*!*************************************************************************
Play sound searches for the audio laoaded into the soumd map, then calls
FMOD API play sound using loaded audio
****************************************************************************/
int CAudioEngine::PlaySound(const std::string& strSoundName, NocturnalEngine::Audio::AudioType chgrp)
{
    // For BGM, check if the same track is already playing
    if (chgrp == NocturnalEngine::Audio::AudioType::BGM)
    {
        if (IsBGMPathPlaying(strSoundName))
        {
            // BGM is already playing, return the existing channel ID
            return mCurrentBGMChannelId;
        }
        else if (!mCurrentBGMPath.empty() && mCurrentBGMPath != strSoundName)
        {
            // Different BGM is being requested, fade out the current one
            FadeOutAndStopChannel(mCurrentBGMChannelId);
            ClearBGMTracking();
        }
    }

    auto tFoundIt = mSoundMap.find(strSoundName);
    FMOD::Sound* pSound;

    if (tFoundIt == mSoundMap.end()) //iterate through channels, if not in any, load sound into a channel
    {
        pSound = Loadsound(strSoundName);
    }
    else
    {
        pSound = tFoundIt->second;
    }

    FMOD::Channel* pChannel = nullptr;

    //create new channel if no empty channels found
    int nChannelId = mNextChannelId++;
    ErrorCheck(mpSystem->playSound(pSound, nullptr, false, &pChannel));
    if (pChannel)
    {
        FMOD_MODE currMode;
        pSound->getMode(&currMode);
        mChannelMap[nChannelId] = pChannel;
        //assign channel group to audio
        if (chgrp == NocturnalEngine::Audio::AudioType::MASTER) {
            pChannel->setChannelGroup(Master);
        }
        if (chgrp == NocturnalEngine::Audio::AudioType::BGM) {
            pChannel->setChannelGroup(BGM);
            // Track the BGM path and channel for persistence
            mCurrentBGMPath = strSoundName;
            mCurrentBGMChannelId = nChannelId;
        }
        if (chgrp == NocturnalEngine::Audio::AudioType::SFX) {
            pChannel->setChannelGroup(SFX);
        }

    }
    return nChannelId;
}

/*!*************************************************************************
Uses FMOD setPaused() to pause audio
****************************************************************************/
void CAudioEngine::PauseSound(int channelID)
{
    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        it->second->setPaused(true);
    }
}

/*!*************************************************************************
Uses FMOD setPaused() and sets to false to unpause audio
****************************************************************************/
void CAudioEngine::UnpauseSound(int channelID)
{
    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        it->second->setPaused(false);
    }
}

/*!*************************************************************************
Stops audio channel groups
****************************************************************************/
void CAudioEngine::StopChannel(channel_groups chan)
{
    if (chan == channel_groups::master)
    {
        Master->stop();
        ClearBGMTracking(); // BGM is part of master
    }

    else if (chan == channel_groups::bgm)
    {
        BGM->stop();
        ClearBGMTracking();
    }

    else if (chan == channel_groups::sfx)
    {
        SFX->stop();
    }
}

/*!*************************************************************************
Sets audio volume
****************************************************************************/
void CAudioEngine::SetVolume(int channelID, float vol)
{
    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        it->second->setVolume(vol);
    }
}

/*!*************************************************************************
Sets audio pan
****************************************************************************/
void CAudioEngine::SetPan(int channelID, float pan)
{
    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        const float clampedPan = std::clamp(pan, -1.0f, 1.0f);
        it->second->setPan(clampedPan);
    }
}

/*!*************************************************************************
Stop all audio channels
****************************************************************************/
void CAudioEngine::StopChannel(int channelID)
{
    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        it->second->stop();
    }
}

/*!*************************************************************************
Returns the current volume level
****************************************************************************/
float CAudioEngine::GetVolume(int channelID)
{
    float audio_volume = 0.0f;

    auto it = mChannelMap.find(channelID);

    if (it != mChannelMap.end())
    {
        it->second->getVolume(&audio_volume);
    }
    return audio_volume;

}

/*!*************************************************************************
Init loop of CAudioEngine
****************************************************************************/
void CAudioEngine::Init()
{
    ErrorCheck(FMOD::System_Create(&mpSystem));

    mpSystem->init(1024, FMOD_INIT_NORMAL, NULL);
    mpSystem->createChannelGroup("Master", &Master);
    mpSystem->createChannelGroup("BGM", &BGM);
    mpSystem->createChannelGroup("SFX", &SFX);

    Master->addGroup(BGM);
    Master->addGroup(SFX);

    // Create DSP effect units and attach to Master channel group (bypassed by default)
    ErrorCheck(mpSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &mDspLowpass));
    ErrorCheck(mpSystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &mDspHighpass));
    ErrorCheck(mpSystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &mDspEcho));
    ErrorCheck(mpSystem->createDSPByType(FMOD_DSP_TYPE_SFXREVERB, &mDspReverb));

    if (mDspLowpass)  { Master->addDSP(0, mDspLowpass);  mDspLowpass->setBypass(true);  }
    if (mDspHighpass) { Master->addDSP(0, mDspHighpass); mDspHighpass->setBypass(true); }
    if (mDspEcho)     { Master->addDSP(0, mDspEcho);     mDspEcho->setBypass(true);     }
    if (mDspReverb)   { Master->addDSP(0, mDspReverb);   mDspReverb->setBypass(true);   }

    //load all audio files from audio folder
    std::string audio_path = "Assets/Audio/";

    if (!std::filesystem::exists(audio_path))
    {
        NOCTURNAL_ENGINE_WARN("Assets/Audio/ folder missing! Creating it...");
        std::filesystem::create_directories(audio_path);
        return;
    }

    for (auto const& dir_entry : std::filesystem::directory_iterator{ audio_path }) //iterate files in Audio folder
    {
        //checks if given file status or path corresponds to a regular file
        if (!dir_entry.is_regular_file())
        {
            continue;
        }
    }
}

/*!*************************************************************************
Update loop of CAudioEngine
****************************************************************************/
void CAudioEngine::Update()
{
    ProcessPendingFadeStops();

    std::vector<std::map<int, FMOD::Channel*>::iterator> pStoppedChannels;
    for (auto it = mChannelMap.begin(); it != mChannelMap.end(); ++it)
    {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);

        if (!bIsPlaying)
        {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto& it : pStoppedChannels)
    {
        mChannelMap.erase(it);
    }
    if (mChannelMap.begin() == mChannelMap.end()) {
        mNextChannelId = 0;
    }
    CAudioEngine::ErrorCheck(mpSystem->update());
}


/*!*************************************************************************
Release loop of CAudioEngine, releases all audio files
****************************************************************************/
void CAudioEngine::Release()
{
    // iterate through mSoundMap
    // release every sound
    for (auto sound_it = mSoundMap.begin(); sound_it != mSoundMap.end(); sound_it++)
    {
        sound_it->second->release();
    }

    // Release DSP effects
    if (mDspLowpass)  { Master->removeDSP(mDspLowpass);  mDspLowpass->release();  mDspLowpass = nullptr;  }
    if (mDspHighpass) { Master->removeDSP(mDspHighpass); mDspHighpass->release(); mDspHighpass = nullptr; }
    if (mDspEcho)     { Master->removeDSP(mDspEcho);     mDspEcho->release();     mDspEcho = nullptr;     }
    if (mDspReverb)   { Master->removeDSP(mDspReverb);   mDspReverb->release();   mDspReverb = nullptr;   }

    // release every channel group
    // release systems
    BGM->stop();
    SFX->stop();
    Master->stop();

    BGM->release();
    SFX->release();
    Master->release();

    mpSystem->release();
}

/*!*************************************************************************
Checks if audio is playing
****************************************************************************/
bool CAudioEngine::IsPlaying(int nChannelId) const
{
    bool is_playing = false;
    auto it = mChannelMap.find(nChannelId);
    //if not found return false
    if (it != mChannelMap.end())
    {
        it->second->isPlaying(&is_playing);
        return is_playing;
    }
    return false;
}
/*!*************************************************************************
Converts decibels to volume
****************************************************************************/
float  CAudioEngine::dbToVolume(float dB)
{
    return powf(10.0f, 0.05f * dB);
}

/*!*************************************************************************
Converts volume to decibels
****************************************************************************/
float  CAudioEngine::VolumeTodB(float volume)
{
    float dec = 20.0f * log10f(volume);

    if (dec > 150.0f)
    {
        //Set cap if volume is maxed
        dec = 149.f;
    }

    return dec;
}

/*!*************************************************************************
Sets looping for audio
****************************************************************************/
void CAudioEngine::SetLooping(const std::string& strSoundName, bool in)
{
    if (in) {
        auto tFoundIt = mSoundMap.find(strSoundName);
        if (tFoundIt != mSoundMap.end()) {
            tFoundIt->second->setMode(FMOD_LOOP_NORMAL);
            tFoundIt->second->setLoopCount(-1);
        }
    }
    else {
        auto tFoundIt = mSoundMap.find(strSoundName);
        if (tFoundIt != mSoundMap.end()) {
            tFoundIt->second->setMode(FMOD_LOOP_OFF);
        }
    }
}

/*!*************************************************************************
Sets volume by channel group
****************************************************************************/
float CAudioEngine::GetVolumeByChannel(FMOD::ChannelGroup* ch) 
{
    float ret = 0;
    ch->getVolume(&ret);
    return ret;
}

/*!*************************************************************************
Checks if a specific BGM path is currently playing
****************************************************************************/
bool CAudioEngine::IsBGMPathPlaying(const std::string& strSoundPath) const
{
    // Check if we have a tracked BGM and if it matches the requested path
    if (mCurrentBGMPath.empty() || mCurrentBGMPath != strSoundPath)
    {
        return false;
    }
    
    // Verify the channel is still actually playing
    if (mCurrentBGMChannelId >= 0)
    {
        auto it = mChannelMap.find(mCurrentBGMChannelId);
        if (it != mChannelMap.end())
        {
            bool isPlaying = false;
            it->second->isPlaying(&isPlaying);
            return isPlaying;
        }
    }
    
    return false;
}

/*!*************************************************************************
Clears BGM tracking (call when BGM should be stopped/reset)
****************************************************************************/
void CAudioEngine::ClearBGMTracking()
{
    mCurrentBGMPath.clear();
    mCurrentBGMChannelId = -1;
}

/*!*************************************************************************
Returns the system sample rate for DSP clock calculations
****************************************************************************/
unsigned long long CAudioEngine::GetSampleRateFromSystem() const
{
    int sampleRate = 0;
    mpSystem->getSoftwareFormat(&sampleRate, nullptr, nullptr);
    return static_cast<unsigned long long>(sampleRate > 0 ? sampleRate : 48000);
}

/*!*************************************************************************
Processes pending fade-out stops, stopping channels whose fade has completed
****************************************************************************/
void CAudioEngine::ProcessPendingFadeStops()
{
    if (mPendingFadeStops.empty())
        return;

    unsigned long long currentClock = 0;
    unsigned long long parentClock = 0;
    if (Master)
        Master->getDSPClock(&currentClock, &parentClock);

    auto it = mPendingFadeStops.begin();
    while (it != mPendingFadeStops.end())
    {
        if (currentClock >= it->stopAtDspClock)
        {
            StopChannel(it->channelId);
            if (mCurrentBGMChannelId == it->channelId)
                ClearBGMTracking();
            it = mPendingFadeStops.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/*!*************************************************************************
Fades out a channel over the specified duration (does not stop it)
****************************************************************************/
void CAudioEngine::FadeOutChannel(int channelID, float fadeDurationSeconds)
{
    auto it = mChannelMap.find(channelID);
    if (it == mChannelMap.end())
        return;

    FMOD::Channel* pChannel = it->second;
    unsigned long long dspclock = 0;
    unsigned long long parentclock = 0;
    pChannel->getDSPClock(&dspclock, &parentclock);

    unsigned long long sampleRate = GetSampleRateFromSystem();
    unsigned long long fadeSamples = static_cast<unsigned long long>(fadeDurationSeconds * sampleRate);

    pChannel->addFadePoint(dspclock, 1.0f);
    pChannel->addFadePoint(dspclock + fadeSamples, 0.0f);
}

/*!*************************************************************************
Fades out a channel over the specified duration, then stops it
****************************************************************************/
void CAudioEngine::FadeOutAndStopChannel(int channelID, float fadeDurationSeconds)
{
    auto it = mChannelMap.find(channelID);
    if (it == mChannelMap.end())
        return;

    FMOD::Channel* pChannel = it->second;
    unsigned long long dspclock = 0;
    unsigned long long parentclock = 0;
    pChannel->getDSPClock(&dspclock, &parentclock);

    unsigned long long sampleRate = GetSampleRateFromSystem();
    unsigned long long fadeSamples = static_cast<unsigned long long>(fadeDurationSeconds * sampleRate);

    pChannel->addFadePoint(dspclock, 1.0f);
    pChannel->addFadePoint(dspclock + fadeSamples, 0.0f);

    FadeOutInfo info{};
    info.channelId = channelID;
    info.stopAtDspClock = dspclock + fadeSamples;
    mPendingFadeStops.push_back(info);
}

/*!*************************************************************************
Fades out all channels in a channel group over the specified duration
****************************************************************************/
void CAudioEngine::FadeOutChannelGroup(channel_groups chan, float fadeDurationSeconds)
{
    FMOD::ChannelGroup* group = nullptr;
    if (chan == channel_groups::master)
        group = Master;
    else if (chan == channel_groups::bgm)
        group = BGM;
    else if (chan == channel_groups::sfx)
        group = SFX;

    if (!group)
        return;

    unsigned long long dspclock = 0;
    unsigned long long parentclock = 0;
    group->getDSPClock(&dspclock, &parentclock);

    unsigned long long sampleRate = GetSampleRateFromSystem();
    unsigned long long fadeSamples = static_cast<unsigned long long>(fadeDurationSeconds * sampleRate);

    group->addFadePoint(dspclock, 1.0f);
    group->addFadePoint(dspclock + fadeSamples, 0.0f);
}

/*!*************************************************************************
Applies audio effect settings to the Master channel group DSP chain
****************************************************************************/
void CAudioEngine::ApplyEffects(const AudioEffectSettings& settings)
{
    mCurrentEffects = settings;

    // Low-pass filter
    if (mDspLowpass)
    {
        mDspLowpass->setBypass(!settings.lowpassEnabled);
        if (settings.lowpassEnabled)
        {
            mDspLowpass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, settings.lowpassCutoff);
            mDspLowpass->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, settings.lowpassResonance);
        }
    }

    // High-pass filter
    if (mDspHighpass)
    {
        mDspHighpass->setBypass(!settings.highpassEnabled);
        if (settings.highpassEnabled)
        {
            mDspHighpass->setParameterFloat(FMOD_DSP_HIGHPASS_CUTOFF, settings.highpassCutoff);
            mDspHighpass->setParameterFloat(FMOD_DSP_HIGHPASS_RESONANCE, settings.highpassResonance);
        }
    }

    // Echo / Delay
    if (mDspEcho)
    {
        mDspEcho->setBypass(!settings.echoEnabled);
        if (settings.echoEnabled)
        {
            mDspEcho->setParameterFloat(FMOD_DSP_ECHO_DELAY, settings.echoDelay);
            mDspEcho->setParameterFloat(FMOD_DSP_ECHO_FEEDBACK, settings.echoFeedback);
            mDspEcho->setParameterFloat(FMOD_DSP_ECHO_DRYLEVEL, settings.echoDryLevel);
            mDspEcho->setParameterFloat(FMOD_DSP_ECHO_WETLEVEL, settings.echoWetLevel);
        }
    }

    // SFX Reverb
    if (mDspReverb)
    {
        mDspReverb->setBypass(!settings.reverbEnabled);
        if (settings.reverbEnabled)
        {
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_DECAYTIME, settings.reverbDecayTime);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_EARLYDELAY, settings.reverbEarlyDelay);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_LATEDELAY, settings.reverbLateDelay);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_HFREFERENCE, settings.reverbHFReference);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_DIFFUSION, settings.reverbDiffusion);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_DENSITY, settings.reverbDensity);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_WETLEVEL, settings.reverbWetLevel);
            mDspReverb->setParameterFloat(FMOD_DSP_SFXREVERB_DRYLEVEL, settings.reverbDryLevel);
        }
    }
}


