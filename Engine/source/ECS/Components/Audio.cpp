/*!------------------------------------------------------------------------------------------------
    File: Audio.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize audio components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the audio class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Audio.h"
#include <imgui.h>

/*!*************************************************************************
Constructor for audio component
****************************************************************************/
NocturnalEngine::Audio::Audio() : AudioArr(), bgmVol{ 1.0f }, sfxVol{ 1.0f }, masterVol{ 1.0f } {}

bool NocturnalEngine::Audio::ToEditor() {
    bool changed = false;

    static std::vector < std::pair<std::string, std::filesystem::directory_entry>> audioFileCache = LoadAudioFromFiles();
    if (audioFileCache.empty()) {
        NOCTURNAL_ENGINE_WARN("NO Audio files in cache!");
        return false;
    }


    if (ImGui::CollapsingHeader("Audio", ImGuiTreeNodeFlags_None))
    {
        auto& AudioArr = GetArr();
        //list out all audio files loaded
        static int currentfile = 0;
        const size_t arraysize = 30;
        const char* audioFileList[arraysize];
        std::vector<const char*> audiofilenames;
        for (std::pair<std::string, std::filesystem::directory_entry>& item : audioFileCache) {
            audiofilenames.push_back(item.first.c_str());
        }
        std::copy(audiofilenames.begin(), audiofilenames.end(), audioFileList);
        ImGui::Text("Loaded Audio Files:");
        changed |= ImGui::Combo("##Audio Files", &currentfile, audioFileList, static_cast<int>(audiofilenames.size()), static_cast<int>(audiofilenames.size()));
        //list out audio file paths in component
        static ImGuiComboFlags flags = 0;
        static int current_audio = 0;
        const size_t aSize = 30;
        const char* AudioList[aSize];
        std::vector<const char*> sList;
        for (int i = 0; i < AudioArr.size(); i++) {
            sList.push_back(AudioArr[i].mAudioPath.c_str());
        }
        std::copy(sList.begin(), sList.end(), AudioList);
        ImGui::Text("Inserted Audio Files:");
        changed |= ImGui::Combo("##Audio In Component", &current_audio, AudioList, static_cast<int>(sList.size()), static_cast<int>(sList.size()));
        //edit channel group
        if (AudioArr.size() > current_audio) {
            ImGui::Text("Inserted Audio Channel Group:");
            const char* ChList = "None\0Master\0BGM\0SFX";
            int ChIndex = static_cast<int>(AudioArr[current_audio].mChannelGroup);
            changed |= ImGui::Combo("###why is this named tes", &ChIndex, ChList);
            ImGui::PushItemWidth(100.0f);
            AudioArr[current_audio].mChannelGroup = static_cast<Audio::AudioType>(ChIndex);
        }
        //option to add audio to array
        if (ImGui::Button("Add Audio")) {
            Audio::AudioPiece nAud;
            nAud.mAudioPath = audioFileList[currentfile];
            nAud.mChannelGroup = Audio::AudioType::NONE;
            nAud.mChannel = 0;
            nAud.is_Looping = false;
            nAud.should_play = false;
            nAud.should_stop = false;
            nAud.is_Playing = false;
            AudioArr.push_back(nAud);
            changed = true;
        }
        ImGui::SameLine();
        //option to remove audio from  
        if (ImGui::Button("Remove Audio")) {
            AudioArr.erase(AudioArr.begin() + current_audio);
            changed = true;
        }

        // Audio Effects section
        ImGui::Separator();
        if (ImGui::TreeNode("Audio Effects"))
        {
            AudioEffectSettings& fx = effectSettings;

            // Low-pass filter
            changed |= ImGui::Checkbox("Low-Pass Filter", &fx.lowpassEnabled);
            if (fx.lowpassEnabled)
            {
                changed |= ImGui::SliderFloat("LP Cutoff (Hz)", &fx.lowpassCutoff, 10.0f, 22000.0f, "%.0f");
                changed |= ImGui::SliderFloat("LP Resonance", &fx.lowpassResonance, 1.0f, 10.0f, "%.1f");
            }

            // High-pass filter
            changed |= ImGui::Checkbox("High-Pass Filter", &fx.highpassEnabled);
            if (fx.highpassEnabled)
            {
                changed |= ImGui::SliderFloat("HP Cutoff (Hz)", &fx.highpassCutoff, 10.0f, 22000.0f, "%.0f");
                changed |= ImGui::SliderFloat("HP Resonance", &fx.highpassResonance, 1.0f, 10.0f, "%.1f");
            }

            // Echo / Delay
            changed |= ImGui::Checkbox("Echo / Delay", &fx.echoEnabled);
            if (fx.echoEnabled)
            {
                changed |= ImGui::SliderFloat("Echo Delay (ms)", &fx.echoDelay, 1.0f, 5000.0f, "%.0f");
                changed |= ImGui::SliderFloat("Echo Feedback (%%)", &fx.echoFeedback, 0.0f, 100.0f, "%.0f");
                changed |= ImGui::SliderFloat("Echo Dry Level (dB)", &fx.echoDryLevel, -80.0f, 10.0f, "%.1f");
                changed |= ImGui::SliderFloat("Echo Wet Level (dB)", &fx.echoWetLevel, -80.0f, 10.0f, "%.1f");
            }

            // Reverb
            changed |= ImGui::Checkbox("Reverb", &fx.reverbEnabled);
            if (fx.reverbEnabled)
            {
                changed |= ImGui::SliderFloat("Decay Time (ms)", &fx.reverbDecayTime, 100.0f, 20000.0f, "%.0f");
                changed |= ImGui::SliderFloat("Early Delay (ms)", &fx.reverbEarlyDelay, 0.0f, 300.0f, "%.1f");
                changed |= ImGui::SliderFloat("Late Delay (ms)", &fx.reverbLateDelay, 0.0f, 100.0f, "%.1f");
                changed |= ImGui::SliderFloat("HF Reference (Hz)", &fx.reverbHFReference, 20.0f, 20000.0f, "%.0f");
                changed |= ImGui::SliderFloat("Diffusion (%%)", &fx.reverbDiffusion, 0.0f, 100.0f, "%.0f");
                changed |= ImGui::SliderFloat("Density (%%)", &fx.reverbDensity, 0.0f, 100.0f, "%.0f");
                changed |= ImGui::SliderFloat("Reverb Wet (dB)", &fx.reverbWetLevel, -80.0f, 20.0f, "%.1f");
                changed |= ImGui::SliderFloat("Reverb Dry (dB)", &fx.reverbDryLevel, -80.0f, 20.0f, "%.1f");
            }

            ImGui::TreePop();
        }
    }
    return changed;
}

/*!*************************************************************************
Deserialize audio component
****************************************************************************/
bool NocturnalEngine::Audio::Deserialize(const rapidjson::Value& obj)
{
	for (int i = 0; i < obj["AudioArrCount"].GetInt(); ++i)
	{
		AudioPiece na{};
		na.mAudioPath = obj[std::to_string(i).c_str()].GetObj()["AudioPath"].GetString();
		na.mChannelGroup = static_cast<AudioType>(obj[std::to_string(i).c_str()].GetObj()["Channel"].GetInt());
		na.is_Looping = obj[std::to_string(i).c_str()].GetObj()["IsLooping"].GetBool();
		na.mChannel = 0;
		na.should_play = false;
		na.should_stop = false;
		na.is_Playing = false;
		na.triggered = false;
		AudioArr.emplace_back(na);
	}
	bgmVol = (float)obj["BGM Volume"].GetDouble();
	sfxVol = (float)obj["SFX Volume"].GetDouble();
	masterVol = (float)obj["Master Volume"].GetDouble();
	if (obj.HasMember("RandomFromArray") && obj["RandomFromArray"].IsBool())randomFromArray = obj["RandomFromArray"].GetBool();

	// Deserialize audio effect settings
	if (obj.HasMember("Effects") && obj["Effects"].IsObject())
	{
		const auto& fx = obj["Effects"].GetObj();
		if (fx.HasMember("LowpassEnabled"))   effectSettings.lowpassEnabled   = fx["LowpassEnabled"].GetBool();
		if (fx.HasMember("LowpassCutoff"))    effectSettings.lowpassCutoff    = (float)fx["LowpassCutoff"].GetDouble();
		if (fx.HasMember("LowpassResonance")) effectSettings.lowpassResonance = (float)fx["LowpassResonance"].GetDouble();

		if (fx.HasMember("HighpassEnabled"))   effectSettings.highpassEnabled   = fx["HighpassEnabled"].GetBool();
		if (fx.HasMember("HighpassCutoff"))    effectSettings.highpassCutoff    = (float)fx["HighpassCutoff"].GetDouble();
		if (fx.HasMember("HighpassResonance")) effectSettings.highpassResonance = (float)fx["HighpassResonance"].GetDouble();

		if (fx.HasMember("EchoEnabled"))  effectSettings.echoEnabled  = fx["EchoEnabled"].GetBool();
		if (fx.HasMember("EchoDelay"))    effectSettings.echoDelay    = (float)fx["EchoDelay"].GetDouble();
		if (fx.HasMember("EchoFeedback")) effectSettings.echoFeedback = (float)fx["EchoFeedback"].GetDouble();
		if (fx.HasMember("EchoDryLevel")) effectSettings.echoDryLevel = (float)fx["EchoDryLevel"].GetDouble();
		if (fx.HasMember("EchoWetLevel")) effectSettings.echoWetLevel = (float)fx["EchoWetLevel"].GetDouble();

		if (fx.HasMember("ReverbEnabled"))     effectSettings.reverbEnabled     = fx["ReverbEnabled"].GetBool();
		if (fx.HasMember("ReverbDecayTime"))   effectSettings.reverbDecayTime   = (float)fx["ReverbDecayTime"].GetDouble();
		if (fx.HasMember("ReverbEarlyDelay"))  effectSettings.reverbEarlyDelay  = (float)fx["ReverbEarlyDelay"].GetDouble();
		if (fx.HasMember("ReverbLateDelay"))   effectSettings.reverbLateDelay   = (float)fx["ReverbLateDelay"].GetDouble();
		if (fx.HasMember("ReverbHFReference")) effectSettings.reverbHFReference = (float)fx["ReverbHFReference"].GetDouble();
		if (fx.HasMember("ReverbDiffusion"))   effectSettings.reverbDiffusion   = (float)fx["ReverbDiffusion"].GetDouble();
		if (fx.HasMember("ReverbDensity"))     effectSettings.reverbDensity     = (float)fx["ReverbDensity"].GetDouble();
		if (fx.HasMember("ReverbWetLevel"))    effectSettings.reverbWetLevel    = (float)fx["ReverbWetLevel"].GetDouble();
		if (fx.HasMember("ReverbDryLevel"))    effectSettings.reverbDryLevel    = (float)fx["ReverbDryLevel"].GetDouble();
	}
	return true;
}



/*!*************************************************************************
Serialise audio component
****************************************************************************/
bool NocturnalEngine::Audio::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
	int i = 0;
	for (i = 0; i < AudioArr.size(); ++i)
	{
		writer->Key(std::to_string(i).c_str());
		writer->StartObject();
		writer->Key("AudioPath");
		writer->String(AudioArr[i].mAudioPath.c_str());
		writer->Key("Channel");
		writer->Int(static_cast<int>(AudioArr[i].mChannelGroup));
		writer->Key("IsLooping");
		writer->Bool(AudioArr[i].is_Looping);
		writer->EndObject();
	}
	writer->Key("AudioArrCount");
	writer->Int(i);
	writer->Key("BGM Volume");
	writer->Double(bgmVol);
	writer->Key("SFX Volume");
	writer->Double(sfxVol);
	writer->Key("Master Volume");
	writer->Double(masterVol);
	writer->Key("RandomFromArray");
	writer->Bool(randomFromArray);

	// Serialize audio effect settings
	writer->Key("Effects");
	writer->StartObject();
	writer->Key("LowpassEnabled");   writer->Bool(effectSettings.lowpassEnabled);
	writer->Key("LowpassCutoff");    writer->Double(effectSettings.lowpassCutoff);
	writer->Key("LowpassResonance"); writer->Double(effectSettings.lowpassResonance);
	writer->Key("HighpassEnabled");   writer->Bool(effectSettings.highpassEnabled);
	writer->Key("HighpassCutoff");    writer->Double(effectSettings.highpassCutoff);
	writer->Key("HighpassResonance"); writer->Double(effectSettings.highpassResonance);
	writer->Key("EchoEnabled");  writer->Bool(effectSettings.echoEnabled);
	writer->Key("EchoDelay");    writer->Double(effectSettings.echoDelay);
	writer->Key("EchoFeedback"); writer->Double(effectSettings.echoFeedback);
	writer->Key("EchoDryLevel"); writer->Double(effectSettings.echoDryLevel);
	writer->Key("EchoWetLevel"); writer->Double(effectSettings.echoWetLevel);
	writer->Key("ReverbEnabled");     writer->Bool(effectSettings.reverbEnabled);
	writer->Key("ReverbDecayTime");   writer->Double(effectSettings.reverbDecayTime);
	writer->Key("ReverbEarlyDelay");  writer->Double(effectSettings.reverbEarlyDelay);
	writer->Key("ReverbLateDelay");   writer->Double(effectSettings.reverbLateDelay);
	writer->Key("ReverbHFReference"); writer->Double(effectSettings.reverbHFReference);
	writer->Key("ReverbDiffusion");   writer->Double(effectSettings.reverbDiffusion);
	writer->Key("ReverbDensity");     writer->Double(effectSettings.reverbDensity);
	writer->Key("ReverbWetLevel");    writer->Double(effectSettings.reverbWetLevel);
	writer->Key("ReverbDryLevel");    writer->Double(effectSettings.reverbDryLevel);
	writer->EndObject();

	return true;
}

std::vector < std::pair<std::string, std::filesystem::directory_entry>> NocturnalEngine::Audio::LoadAudioFromFiles(){

    std::vector<std::pair<std::string, std::filesystem::directory_entry>> returnVal{};
    std::string audio_path = "Assets/Audio";

    if (!std::filesystem::exists(audio_path))
    {
        NOCTURNAL_ENGINE_WARN("Assets/Audio/ folder missing! Creating it...");
        std::filesystem::create_directories(audio_path);
        return returnVal; // Nothing to load
    }

    for (auto const& dir_entry : std::filesystem::directory_iterator{ audio_path }) //iterate files in Audio folder
    {
        //checks if given file status or path corresponds to a regular file
        if (!dir_entry.is_regular_file())
        {
            continue;
        }

        returnVal.emplace_back(dir_entry.path().filename().string(),dir_entry);

    }
    return returnVal;
}
