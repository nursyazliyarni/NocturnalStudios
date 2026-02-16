/*!------------------------------------------------------------------------------------------------
    File: IGameConfig.h
    Project: NocturnalStudio

    Brief:
        This file enabled injection of a game into0 notcurnalengine

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the IGameLayer class and its rerspective functoiond


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"

namespace NocturnalEngine {
    class NOCTURNAL_ENGINE_API IGameConfig
    {
    public:
        virtual ~IGameConfig() = default;

        // Check if a scene is a playable gameplay scene
        virtual bool IsPlayableScene(const std::string& sceneName) const = 0;

        // Called when entering play mode
        virtual void OnEnterPlayMode(const std::string& currentScene) = 0;

        // Called when exiting play mode
        virtual void OnExitPlayMode(bool ) = 0;

        
    };

}
