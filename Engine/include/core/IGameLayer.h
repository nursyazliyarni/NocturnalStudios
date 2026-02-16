/*!------------------------------------------------------------------------------------------------
    File: IGameLayer.h
    Project: NocturnalStudio

    Brief:
        This file enabled injection of a game into0 notcurnalengine

    Primary Author:
         LAU XIANG YANG (87%)
        - Created the IGameLayer class and its rerspective functoiond


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
#pragma once
namespace NocturnalEngine {
    class  IGameLayer
    {
    public:
        virtual void Init() = 0;
        virtual void Update(float dt) = 0;
        virtual void Shutdown() = 0;

        virtual void RegisterScripts() = 0;
    };

}
