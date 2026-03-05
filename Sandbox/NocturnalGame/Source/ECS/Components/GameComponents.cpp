/*!------------------------------------------------------------------------------------------------
    File: GameComponents.cpp
    Project: NocturnalStudio

    Brief: declare the game components here, like Components.cpp on the engine side but for the
    game specific ones


    Primary Author:
         LAYCOCK ENHUI SARAH (100%)
        - Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/GameComponentReg.h"
#include "ECS/Components/GameComponents.h"



namespace NocturnalEngine {
    void GameComponentRegistry::RegisterAllGameComponents() {
        //AnimalAttributes
        RegisterGameComponentInECS<AnimalAttributes>();
        RegisterComponent<AnimalAttributes>("AnimalAttributes");
        //ItemsAttributes
        RegisterGameComponentInECS<ItemsAttributes>();
        RegisterComponent<ItemsAttributes>("ItemsAttributes");
        //ButtonComponent
        RegisterGameComponentInECS<ButtonComponent>();
        RegisterComponent<ButtonComponent>("ButtonComponent");

    }
}
