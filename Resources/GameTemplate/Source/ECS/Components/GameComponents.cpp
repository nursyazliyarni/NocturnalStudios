/*!------------------------------------------------------------------------------------------------
    File: GameComponents.cpp
    Project: TEMPLATE_PROJECT_NAME

    Brief:


    Primary Author:


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
