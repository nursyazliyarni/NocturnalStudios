/*!------------------------------------------------------------------------------------------------
    File: Components.h
    Project: NocturnalStudio

	Brief:
        This file contains all the Various Components to be used in the ECS
        in the future. Currently, Every Component is still handling its own data
        by inheriting from an abstract class of IComponent to serialize and
        deserialize its data
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Components.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Transform.h"
#include "RigidBody.h"
#include "WinData.h"
#include "Collider.h"
#include "NameTag.h"
#include "Sprite.h"
#include "Logic.h"
#include "Tag.h"
#include "Audio.h"
#include "HUDComponent.h"
#include "AnimalAttributes.h"
#include "ItemsAttributes.h"



//Macros for creating default values of various components
#define C_TransformComponent NocturnalEngine::Transform::Transform()
#define C_RigidBodyComponent NocturnalEngine::RigidBody::RigidBody()
#define C_ColliderComponent NocturnalEngine::Collider::Collider()
#define C_NameTagComponent NocturnalEngine::NameTag::NameTag()
#define C_SpriteComponent NocturnalEngine::Sprite::Sprite()
#define C_LogicComponent NocturnalEngine::Logic::Logic()
#define C_TagComponent NocturnalEngine::Tag::Tag()
#define C_AudioComponent NocturnalEngine::Audio::Audio()
#define C_HUDComponent NocturnalEngine::HUDComponent::HUDComponent()
#define C_AnimalAttributesComponent NocturnalEngine::AnimalAttributes::AnimalAttributes()
#define C_ItemsAttributesComponent NocturnalEngine::ItemsAttributes::ItemsAttributes()
