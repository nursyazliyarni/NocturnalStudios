/*!------------------------------------------------------------------------------------------------
    File: Video.cpp
    Project: NocturnalStudio

    Brief:
        This file is used to serialize and deserialize video components for ECS.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Video.h"

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <cstdio>

namespace NocturnalEngine
{
    namespace
    {
        struct SkipKeyOption
        {
            int key;
            const char* label;
        };

        constexpr SkipKeyOption kSkipKeyOptions[] = {
            { GLFW_KEY_SPACE, "Space" },
            { GLFW_KEY_ENTER, "Enter" },
            { GLFW_KEY_ESCAPE, "Escape" },
            { GLFW_KEY_S, "S" },
            { GLFW_KEY_X, "X" }
        };
    }

    bool Video::Deserialize(const rapidjson::Value& obj)
    {
        if (obj.HasMember("VideoPath") && obj["VideoPath"].IsString()) mVideoPath = obj["VideoPath"].GetString();
        if (obj.HasMember("Enabled") && obj["Enabled"].IsBool()) enabled = obj["Enabled"].GetBool();
        if (obj.HasMember("Loop") && obj["Loop"].IsBool()) loop = obj["Loop"].GetBool();
        if (obj.HasMember("PlayInEditor") && obj["PlayInEditor"].IsBool()) playInEditor = obj["PlayInEditor"].GetBool();
        if (obj.HasMember("AllowSkip") && obj["AllowSkip"].IsBool()) allowSkip = obj["AllowSkip"].GetBool();
        if (obj.HasMember("SkipOnMouseClick") && obj["SkipOnMouseClick"].IsBool()) skipOnMouseClick = obj["SkipOnMouseClick"].GetBool();
        if (obj.HasMember("RestartOnEnable") && obj["RestartOnEnable"].IsBool()) restartOnEnable = obj["RestartOnEnable"].GetBool();
        if (obj.HasMember("HideWhenFinished") && obj["HideWhenFinished"].IsBool()) hideWhenFinished = obj["HideWhenFinished"].GetBool();
        if (obj.HasMember("SkipKey") && obj["SkipKey"].IsInt()) skipKey = obj["SkipKey"].GetInt();
        if (obj.HasMember("NextScene") && obj["NextScene"].IsString()) nextScene = obj["NextScene"].GetString();
        if (obj.HasMember("AudioPath") && obj["AudioPath"].IsString()) audioPath = obj["AudioPath"].GetString();
        if (obj.HasMember("AudioChannelGroup") && obj["AudioChannelGroup"].IsInt()) audioChannelGroup = static_cast<Audio::AudioType>(obj["AudioChannelGroup"].GetInt());
        if (obj.HasMember("AudioLoop") && obj["AudioLoop"].IsBool()) audioLoop = obj["AudioLoop"].GetBool();
        return true;
    }

    bool Video::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
    {
        writer->Key("VideoPath");
        writer->String(mVideoPath.c_str());
        writer->Key("Enabled");
        writer->Bool(enabled);
        writer->Key("Loop");
        writer->Bool(loop);
        writer->Key("PlayInEditor");
        writer->Bool(playInEditor);
        writer->Key("AllowSkip");
        writer->Bool(allowSkip);
        writer->Key("SkipOnMouseClick");
        writer->Bool(skipOnMouseClick);
        writer->Key("RestartOnEnable");
        writer->Bool(restartOnEnable);
        writer->Key("HideWhenFinished");
        writer->Bool(hideWhenFinished);
        writer->Key("SkipKey");
        writer->Int(skipKey);
        writer->Key("NextScene");
        writer->String(nextScene.c_str());
        writer->Key("AudioPath");
        writer->String(audioPath.c_str());
        writer->Key("AudioChannelGroup");
        writer->Int(static_cast<int>(audioChannelGroup));
        writer->Key("AudioLoop");
        writer->Bool(audioLoop);
        return true;
    }

    bool Video::ToEditor()
    {
        bool changed = false;
        static std::vector<std::pair<std::string, std::filesystem::directory_entry>> videoFileCache = LoadVideosFromFiles();
        static std::vector<std::pair<std::string, std::filesystem::directory_entry>> audioFileCache = LoadAudioFromFiles();

        if (ImGui::CollapsingHeader("Video", ImGuiTreeNodeFlags_None))
        {
            if (ImGui::Button("Refresh Video List"))
            {
                videoFileCache = LoadVideosFromFiles();
                audioFileCache = LoadAudioFromFiles();
            }

            changed |= ImGui::Checkbox("Enabled", &enabled);
            changed |= ImGui::Checkbox("Loop", &loop);
            changed |= ImGui::Checkbox("Play In Editor", &playInEditor);
            changed |= ImGui::Checkbox("Allow Skip", &allowSkip);
            changed |= ImGui::Checkbox("Skip On Mouse Click", &skipOnMouseClick);
            changed |= ImGui::Checkbox("Restart When Re-Enabled", &restartOnEnable);
            changed |= ImGui::Checkbox("Hide When Finished", &hideWhenFinished);

            ImGui::Text("Current Video: %s", mVideoPath.empty() ? "<none>" : mVideoPath.c_str());

            if (!videoFileCache.empty())
            {
                int currentIndex = 0;
                for (std::size_t i = 0; i < videoFileCache.size(); ++i)
                {
                    if (videoFileCache[i].first == mVideoPath)
                    {
                        currentIndex = static_cast<int>(i);
                        break;
                    }
                }

                std::vector<const char*> items;
                items.reserve(videoFileCache.size());
                for (const auto& item : videoFileCache)
                {
                    items.push_back(item.first.c_str());
                }

                if (ImGui::Combo("Video File", &currentIndex, items.data(), static_cast<int>(items.size())))
                {
                    mVideoPath = videoFileCache[currentIndex].first;
                    changed = true;
                }
            }
            else
            {
                ImGui::Text("No videos found in Assets/Video.");
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Videos"))
                {
                    mVideoPath = static_cast<const char*>(payload->Data);
                    changed = true;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::Separator();
            ImGui::Text("Attached Audio");
            ImGui::Text("Current Audio: %s", audioPath.empty() ? "<none>" : audioPath.c_str());

            if (!audioFileCache.empty())
            {
                int currentAudioIndex = 0;
                for (std::size_t i = 0; i < audioFileCache.size(); ++i)
                {
                    if (audioFileCache[i].first == audioPath)
                    {
                        currentAudioIndex = static_cast<int>(i);
                        break;
                    }
                }

                std::vector<const char*> audioItems;
                audioItems.reserve(audioFileCache.size());
                for (const auto& item : audioFileCache)
                {
                    audioItems.push_back(item.first.c_str());
                }

                if (ImGui::Combo("Audio File", &currentAudioIndex, audioItems.data(), static_cast<int>(audioItems.size())))
                {
                    audioPath = audioFileCache[currentAudioIndex].first;
                    changed = true;
                }
            }
            else
            {
                ImGui::Text("No audio files found in Assets/Audio.");
            }

            if (ImGui::Button("Clear Attached Audio"))
            {
                audioPath.clear();
                changed = true;
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AudioFiles"))
                {
                    audioPath = static_cast<const char*>(payload->Data);
                    changed = true;
                }
                ImGui::EndDragDropTarget();
            }

            const char* channelLabels = "None\0Master\0BGM\0SFX\0";
            int channelIndex = static_cast<int>(audioChannelGroup);
            if (ImGui::Combo("Audio Channel", &channelIndex, channelLabels))
            {
                audioChannelGroup = static_cast<Audio::AudioType>(channelIndex);
                changed = true;
            }
            changed |= ImGui::Checkbox("Loop Attached Audio", &audioLoop);

            int currentSkipKey = 0;
            for (int i = 0; i < static_cast<int>(std::size(kSkipKeyOptions)); ++i)
            {
                if (kSkipKeyOptions[i].key == skipKey)
                {
                    currentSkipKey = i;
                    break;
                }
            }

            std::vector<const char*> keyLabels;
            for (const auto& option : kSkipKeyOptions)
            {
                keyLabels.push_back(option.label);
            }

            if (ImGui::Combo("Skip Key", &currentSkipKey, keyLabels.data(), static_cast<int>(keyLabels.size())))
            {
                skipKey = kSkipKeyOptions[currentSkipKey].key;
                changed = true;
            }

            char nextSceneBuffer[256]{};
            std::snprintf(nextSceneBuffer, sizeof(nextSceneBuffer), "%s", nextScene.c_str());
            if (ImGui::InputText("Next Scene", nextSceneBuffer, sizeof(nextSceneBuffer)))
            {
                nextScene = nextSceneBuffer;
                changed = true;
            }
            ImGui::TextWrapped("Use a scene path like Assets/Scene/CutScene.json. Leave empty to stay in the current scene.");
        }

        return changed;
    }

    std::vector<std::pair<std::string, std::filesystem::directory_entry>> Video::LoadVideosFromFiles() const
    {
        std::vector<std::pair<std::string, std::filesystem::directory_entry>> videos;
        const std::filesystem::path videoPath = "Assets/Video";

        if (!std::filesystem::exists(videoPath))
        {
            std::filesystem::create_directories(videoPath);
            return videos;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(videoPath))
        {
            if (!entry.is_regular_file())
                continue;

            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext != ".mpg" && ext != ".mpeg")
                continue;

            const std::string relativePath = std::filesystem::relative(entry.path(), videoPath).generic_string();
            videos.emplace_back(relativePath, entry);
        }

        std::sort(videos.begin(), videos.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.first < rhs.first;
        });

        return videos;
    }

    std::vector<std::pair<std::string, std::filesystem::directory_entry>> Video::LoadAudioFromFiles() const
    {
        std::vector<std::pair<std::string, std::filesystem::directory_entry>> audioFiles;
        const std::filesystem::path audioPathRoot = "Assets/Audio";

        if (!std::filesystem::exists(audioPathRoot))
        {
            std::filesystem::create_directories(audioPathRoot);
            return audioFiles;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(audioPathRoot))
        {
            if (!entry.is_regular_file())
                continue;

            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext != ".wav" && ext != ".ogg" && ext != ".mp3")
                continue;

            const std::string relativePath = std::filesystem::relative(entry.path(), audioPathRoot).generic_string();
            audioFiles.emplace_back(relativePath, entry);
        }

        std::sort(audioFiles.begin(), audioFiles.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.first < rhs.first;
        });

        return audioFiles;
    }

}
