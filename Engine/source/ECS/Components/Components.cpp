/*!------------------------------------------------------------------------------------------------
    File: Components.cpp
    Project: NocturnalStudio

	Brief: This file contains the declaration of the EngineComponentRegistry's RegisterAllEngineComponents
            function, which provides the registry with the information of the engine;s components
		
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the file.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/EngineComponentReg.h"

namespace NocturnalEngine{
    void EngineComponentRegistry::RegisterAllEngineComponents() {
        //Transform
        p_ecs.RegisterComponent<Transform>(); //0
        RegisterComponent<Transform>("Transform");
        //RigidBody
        p_ecs.RegisterComponent<RigidBody>();//1
        RegisterComponent<RigidBody>("RigidBody");
        //Collider
        p_ecs.RegisterComponent<Collider>();//2
        RegisterComponent<Collider>("Collider");
        //NameTag
        p_ecs.RegisterComponent<NameTag>();//3
        RegisterComponent<NameTag>("NameTag");
        //Sprite
        p_ecs.RegisterComponent<Sprite>();//4
        RegisterComponent<Sprite>("Sprite");
        //Logic
        p_ecs.RegisterComponent<Logic>();//5
        RegisterComponent<Logic>("Logic");
        //Tag
        p_ecs.RegisterComponent<Tag>();//6
        RegisterComponent<Tag>("Tag");
        //Audio
        p_ecs.RegisterComponent<Audio>();//7
        RegisterComponent<Audio>("Audio");
        //Video
        p_ecs.RegisterComponent<Video>();//8
        RegisterComponent<Video>("Video");
        //HUDComponent
        p_ecs.RegisterComponent<HUDComponent>();//9
        RegisterComponent<HUDComponent>("HUDComponent");
        //Draggable
        p_ecs.RegisterComponent<Draggable>();//10
        RegisterComponent<Draggable>("Draggable");
        //Text
        p_ecs.RegisterComponent<Text>();//11
        RegisterComponent<Text>("Text");

        //SceneVFX
        p_ecs.RegisterComponent<SceneVFX>();//12
        RegisterComponent<SceneVFX>("SceneVFX");


    }
}
