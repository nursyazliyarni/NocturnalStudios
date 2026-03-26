/*!------------------------------------------------------------------------------------------------
    File: VideoPlayback.cpp
    Project: NocturnalStudio

    Brief:
        Runtime MPEG video decoding and texture upload helpers.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Video/VideoPlayback.h"

#include "ECS/Components/Video.h"
#include "ECS/SceneManager.h"
#include "Input/Input.h"
#include "ResourceManager/ResourceManager.h"
#include <cstdio>
#include "../../../lib/pl_mpeg/pl_mpeg.h"

#include <GLFW/glfw3.h>

namespace NocturnalEngine
{
    namespace
    {
        std::filesystem::path ResolveAssetPath(const std::filesystem::path& subdirectory, const std::string& relativePath)
        {
            const std::filesystem::path requestedPath{ relativePath };
            if (requestedPath.is_absolute() && std::filesystem::exists(requestedPath))
                return requestedPath;

            const std::filesystem::path cwd = std::filesystem::current_path();
            std::vector<std::filesystem::path> candidates{
                cwd / "Assets" / subdirectory / requestedPath,
                cwd / "Sandbox" / "NocturnalGame" / "Assets" / subdirectory / requestedPath,
                cwd / "build" / "Sandbox" / "NocturnalGame" / "Debug" / "Assets" / subdirectory / requestedPath,
                cwd / "build" / "Sandbox" / "NocturnalGame" / "Release" / "Assets" / subdirectory / requestedPath
            };

            for (std::filesystem::path search = cwd; !search.empty() && search != search.root_path(); search = search.parent_path())
            {
                candidates.push_back(search / "Assets" / subdirectory / requestedPath);
                candidates.push_back(search / "Sandbox" / "NocturnalGame" / "Assets" / subdirectory / requestedPath);
                candidates.push_back(search / "build" / "Sandbox" / "NocturnalGame" / "Debug" / "Assets" / subdirectory / requestedPath);
                candidates.push_back(search / "build" / "Sandbox" / "NocturnalGame" / "Release" / "Assets" / subdirectory / requestedPath);
            }

            for (const auto& candidate : candidates)
            {
                if (std::filesystem::exists(candidate))
                    return candidate;
            }

            return {};
        }

        struct VideoRuntimeState
        {
            plm_t* decoder{ nullptr };
            std::shared_ptr<Texture> texture{};
            std::vector<uint8_t> rgbaBuffer{};
            std::string loadedPath{};
            std::string loadedAudioPath{};
            int audioChannelId{ -1 };
            bool completed{ false };
        };

        std::unordered_map<Entity, VideoRuntimeState> gVideoRuntimeStates;

        void UploadFrame(plm_t* plm, plm_frame_t* frame, void* user)
        {
            UNREFERENCED_PARAMETER(plm);
            if (!frame || !user)
                return;

            auto* state = static_cast<VideoRuntimeState*>(user);
            if (!state->texture || frame->width == 0 || frame->height == 0)
                return;

            const std::size_t requiredSize = static_cast<std::size_t>(frame->width) * static_cast<std::size_t>(frame->height) * 4u;
            if (state->rgbaBuffer.size() != requiredSize)
            {
                state->rgbaBuffer.resize(requiredSize);
            }

            std::vector<uint8_t> decodedBuffer(requiredSize);
            plm_frame_to_rgba(frame, decodedBuffer.data(), static_cast<int>(frame->width * 4u));

            const std::size_t rowSize = static_cast<std::size_t>(frame->width) * 4u;
            for (int row = 0; row < frame->height; ++row)
            {
                const std::size_t srcOffset = static_cast<std::size_t>(frame->height - 1 - row) * rowSize;
                const std::size_t dstOffset = static_cast<std::size_t>(row) * rowSize;
                std::memcpy(state->rgbaBuffer.data() + dstOffset, decodedBuffer.data() + srcOffset, rowSize);
            }

            for (std::size_t alphaIndex = 3; alphaIndex < state->rgbaBuffer.size(); alphaIndex += 4)
            {
                state->rgbaBuffer[alphaIndex] = 255;
            }
            state->texture->SetTextureData(state->rgbaBuffer.data(), static_cast<unsigned int>(state->rgbaBuffer.size()));
        }

        void DestroyState(VideoRuntimeState& state)
        {
            if (state.decoder)
            {
                plm_destroy(state.decoder);
                state.decoder = nullptr;
            }
            state.texture.reset();
            state.rgbaBuffer.clear();
            state.loadedPath.clear();
            state.loadedAudioPath.clear();
            if (state.audioChannelId >= 0)
            {
                p_Audio->FadeOutAndStopChannel(state.audioChannelId);
                state.audioChannelId = -1;
            }
            state.completed = false;
        }

        void StopAttachedAudio(VideoRuntimeState& state)
        {
            if (state.audioChannelId >= 0)
            {
                p_Audio->FadeOutAndStopChannel(state.audioChannelId);
                state.audioChannelId = -1;
            }
            state.loadedAudioPath.clear();
        }

        void StartAttachedAudio(const Video& video, VideoRuntimeState& state)
        {
            if (video.audioPath.empty())
            {
                StopAttachedAudio(state);
                return;
            }

            const std::filesystem::path fullAudioPath = ResolveAssetPath("Audio", video.audioPath);
            if (fullAudioPath.empty())
            {
                StopAttachedAudio(state);
                NOCTURNAL_ENGINE_WARN("Attached video audio file not found: {}", video.audioPath);
                return;
            }

            const std::string fullPathString = fullAudioPath.generic_string();
            if (state.audioChannelId >= 0 && state.loadedAudioPath == fullPathString && p_Audio->IsPlaying(state.audioChannelId))
                return;

            StopAttachedAudio(state);
            p_Audio->Loadsound(fullPathString, video.audioLoop);
            state.audioChannelId = p_Audio->PlaySound(fullPathString, video.audioChannelGroup);
            state.loadedAudioPath = fullPathString;
        }

        bool LoadState(Entity entity, Video& video, VideoRuntimeState& state)
        {
            DestroyState(state);
            video.finished = false;

            if (video.GetVideoPath().empty())
                return false;

            const std::filesystem::path fullPath = ResolveAssetPath("Video", video.GetVideoPath());
            if (fullPath.empty())
            {
                NOCTURNAL_ENGINE_WARN("Video file not found: {}", video.GetVideoPath());
                return false;
            }

            state.decoder = plm_create_with_filename(fullPath.string().c_str());
            if (!state.decoder || !plm_has_headers(state.decoder))
            {
                NOCTURNAL_ENGINE_WARN("Failed to open MPEG video: {}", fullPath.string());
                DestroyState(state);
                return false;
            }

            plm_set_audio_enabled(state.decoder, FALSE);
            plm_set_loop(state.decoder, video.loop ? TRUE : FALSE);
            plm_set_video_decode_callback(state.decoder, UploadFrame, &state);

            const int width = plm_get_width(state.decoder);
            const int height = plm_get_height(state.decoder);
            if (width <= 0 || height <= 0)
            {
                DestroyState(state);
                return false;
            }

            state.texture = ResourceManager::GenTexture(static_cast<unsigned int>(width), static_cast<unsigned int>(height));
            state.rgbaBuffer.resize(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u);
            state.loadedPath = video.GetVideoPath();

            plm_frame_t* firstFrame = plm_decode_video(state.decoder);
            if (firstFrame)
            {
                UploadFrame(state.decoder, firstFrame, &state);
            }
            else
            {
                plm_rewind(state.decoder);
            }

            NOCTURNAL_ENGINE_INFO("Loaded video '{}' for entity {}", fullPath.string(), entity);
            return true;
        }

        bool SkipRequested(const Video& video)
        {
            if (!video.allowSkip)
                return false;
            if (p_Input->KeyPressed(video.skipKey))
                return true;
            if (video.skipOnMouseClick && p_Input->MousePressed(GLFW_MOUSE_BUTTON_LEFT))
                return true;
            return false;
        }

        void HandleVideoFinished(Video& video, VideoRuntimeState& state)
        {
            if (state.completed)
                return;

            state.completed = true;
            video.finished = true;
            StopAttachedAudio(state);
            if (!video.nextScene.empty())
            {
                p_Scene->setSceneToLoad(video.nextScene);
            }
        }
    }

    std::shared_ptr<Texture> VideoPlayback::UpdateAndGetTexture(Entity entity, Video& video, float deltaTime, bool editorActive)
    {
        auto& state = gVideoRuntimeStates[entity];

        if (!video.enabled || video.GetVideoPath().empty())
        {
            video.finished = false;
            if (video.restartOnEnable)
                DestroyState(state);
            return nullptr;
        }

        if (state.loadedPath != video.GetVideoPath() || !state.decoder || !state.texture)
        {
            if (!LoadState(entity, video, state))
                return nullptr;
        }

        const bool shouldAdvancePlayback = !editorActive || video.playInEditor;
        if (!shouldAdvancePlayback)
        {
            StopAttachedAudio(state);
            return state.texture;
        }

        if (!state.completed)
        {
            StartAttachedAudio(video, state);
        }

        if (!editorActive && SkipRequested(video))
        {
            HandleVideoFinished(video, state);
            if (video.hideWhenFinished)
                return nullptr;
            return state.texture;
        }

        if (!state.completed)
        {
            plm_set_loop(state.decoder, video.loop ? TRUE : FALSE);
            plm_decode(state.decoder, static_cast<double>(deltaTime));

            if (!video.loop && plm_has_ended(state.decoder))
            {
                HandleVideoFinished(video, state);
            }
        }

        if (state.completed && video.hideWhenFinished)
            return nullptr;

        return state.texture;
    }

    void VideoPlayback::CleanupMissing(const std::set<Entity>& aliveEntities)
    {
        for (auto it = gVideoRuntimeStates.begin(); it != gVideoRuntimeStates.end();)
        {
            if (!aliveEntities.contains(it->first))
            {
                DestroyState(it->second);
                it = gVideoRuntimeStates.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void VideoPlayback::Shutdown()
    {
        for (auto& [entity, state] : gVideoRuntimeStates)
        {
            UNREFERENCED_PARAMETER(entity);
            DestroyState(state);
        }
        gVideoRuntimeStates.clear();
    }
}
