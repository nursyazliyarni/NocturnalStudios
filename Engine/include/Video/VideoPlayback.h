/*!------------------------------------------------------------------------------------------------
    File: VideoPlayback.h
    Project: NocturnalStudio

    Brief:
        Runtime MPEG video decoding and texture upload helpers.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Graphics/Texture.h"
#include "ECS/Types.h"

namespace NocturnalEngine
{
    class Video;

    class VideoPlayback
    {
    public:
        static std::shared_ptr<Texture> UpdateAndGetTexture(Entity entity, Video& video, float deltaTime, bool editorActive);
        static void CleanupMissing(const std::set<Entity>& aliveEntities);
        static void Shutdown();
    };
}
