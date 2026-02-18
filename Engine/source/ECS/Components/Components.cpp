/*!------------------------------------------------------------------------------------------------
    File: Components.cpp
    Project: NocturnalStudio

	Brief:
		
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/EngineComponentReg.h"

namespace NocturnalEngine{
    void EngineComponentRegistry::RegisterAllEngineComponents() {
        RegisterComponent<Transform>("Transform");
        RegisterComponent<RigidBody>("RigidBody");
        RegisterComponent<Collider>("Collider");
        RegisterComponent<NameTag>("NameTag");
        RegisterComponent<Sprite>("Sprite");
        RegisterComponent<Tag>("Tag");
        RegisterComponent<Audio>("Audio");
        RegisterComponent<Text>("Text");
        RegisterComponent<HUDComponent>("HUDComponent");
        RegisterComponent<AnimalAttributes>("AnimalAttributes");
        RegisterComponent<ItemsAttributes>("ItemsAttributes");
        RegisterComponent<Draggable>("Draggable");
        RegisterComponent<Logic>("Logic");
    }
}
