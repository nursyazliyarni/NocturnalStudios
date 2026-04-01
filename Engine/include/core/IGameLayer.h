/*!------------------------------------------------------------------------------------------------
    File: IGameLayer.h
    Project: NocturnalStudio

    Brief:
        This file enables the injection of a game into the engine

    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the IGameLayer class and its rerspective functions


    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "pch.h"
#pragma once
namespace NocturnalEngine {
    /*!*************************************************************************
    Used by the engine to run the game
    ****************************************************************************/
    class Window;
    class  IGameLayer
    {
    public:
        /*!*************************************************************************
        Defined on the game side
        ****************************************************************************/
        virtual void Init() = 0;
        virtual void Update(float dt) = 0;
        virtual void InputUpdate(float dt) { UNREFERENCED_PARAMETER(dt); }
        virtual void Shutdown() = 0;

        /*!*************************************************************************
        Idk if i even need this
        ****************************************************************************/
        virtual void RegisterScripts() = 0;
        virtual void GiveWindow(Window* window) { UNREFERENCED_PARAMETER(window); }
    };

}
