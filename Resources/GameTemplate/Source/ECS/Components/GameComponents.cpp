/*!------------------------------------------------------------------------------------------------
    File: GameComponents.cpp
    Project: NocturnalStudio

    Brief:


    Primary Author:
         LAU XIANG YANG (100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/GameComponentReg.h"
#include "ECS/Components/GameComponents.h"



namespace NocturnalEngine {
    void GameComponentRegistry::RegisterAllGameComponents() {
        RegisterGameComponentInECS<GameComponentTemplate>();
        RegisterComponent<GameComponentTemplate>("GameComponentTemplate");
    }
}
