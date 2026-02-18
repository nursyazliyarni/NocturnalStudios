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

void NocturnalEngine::Audio::ToEditor() {

    static std::vector < std::pair<std::string, std::filesystem::directory_entry>> audioFileCache = LoadAudioFromFiles();
    if (audioFileCache.empty()) {
        NOCTURNAL_ENGINE_WARN("NO Audio files in cache!");
        return;
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
        ImGui::Combo("##Audio Files", &currentfile, audioFileList, static_cast<int>(audiofilenames.size()), static_cast<int>(audiofilenames.size()));
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
        ImGui::Combo("##Audio In Component", &current_audio, AudioList, static_cast<int>(sList.size()), static_cast<int>(sList.size()));
        //edit channel group
        if (AudioArr.size() > current_audio) {
            ImGui::Text("Inserted Audio Channel Group:");
            const char* ChList = "None\0Master\0BGM\0SFX";
            int ChIndex = static_cast<int>(AudioArr[current_audio].mChannelGroup);
            ImGui::Combo("###why is this named tes", &ChIndex, ChList);
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
        }
        ImGui::SameLine();
        //option to remove audio from array
        if (ImGui::Button("Remove Audio")) {
            AudioArr.erase(AudioArr.begin() + current_audio);
        }
    }
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
