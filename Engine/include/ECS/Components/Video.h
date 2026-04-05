/*!------------------------------------------------------------------------------------------------
    File: Video.h
    Project: NocturnalStudio

    Brief:
        Contains the component related to MPEG video playback in ECS.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "IComponent.h"
#include "Audio.h"
#include <vector>
#include <filesystem>

namespace NocturnalEngine
{
    class Video : public IComponent
    {
    public:
        Video() = default;
        ~Video() = default;

        virtual bool Deserialize(const rapidjson::Value& obj) override;
        virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const override;
        virtual bool ToEditor() override;

        Entity& GetComponentEntityID() override { return entityID; }
        void SetComponentEntityID(Entity& entity) override { entityID = entity; }

        const std::string& GetVideoPath() const { return mVideoPath; }
        void SetVideoPath(const std::string& path) { mVideoPath = path; }

        bool enabled{ true };
        bool loop{ false };
        bool playInEditor{ false };
        bool allowSkip{ true };
        bool skipOnMouseClick{ false };
        bool restartOnEnable{ true };
        bool hideWhenFinished{ false };
        bool finished{ false };
        int skipKey{ 32 };
        std::string nextScene{};
        std::string audioPath{};
        Audio::AudioType audioChannelGroup{ Audio::AudioType::MASTER };
        bool audioLoop{ false };
        float audioStartDelay{ 0.0f };

    private:
        std::vector<std::pair<std::string, std::filesystem::directory_entry>> LoadVideosFromFiles() const;
        std::vector<std::pair<std::string, std::filesystem::directory_entry>> LoadAudioFromFiles() const;

        std::string mVideoPath{};
    };
}
