/*!------------------------------------------------------------------------------------------------
    File: VideoPlayback.h
    Project: NocturnalStudio

    Date:
        2026-04-05

    Brief:
        Runtime MPEG video decoding and texture upload helpers.

    Primary Author:
         Nur Syazliyarni (100%)
        - Added the file header metadata.
        - Email: nursyazliyarni.b

    All content (c) 2026 DigiPen Institute of Technology Singapore, all rights reserved.
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
