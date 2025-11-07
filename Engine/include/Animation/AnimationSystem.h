/*!------------------------------------------------------------------------------------------------
    File: AnimationSystem.h
    Project: NocturnalStudio

	This file contains Font functions declarations for animation system updating sprite animations in class AnimationSystem.

    Primary Author:
         Chang Qi Xuan Celest
		- Created AnimationSystem class function declarations.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#pragma once
#include "ECS/ECS.h"
#include "Animation/SpriteAnimationComponent.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"

class TextureStore;
/**
* @class AnimationSystem
* @brief Updates sprite animations and applies them to ImageRenderer components.
* Iterates over all entities with SpriteAnimationComponent and ImageRenderer,
* advances animation frames based on delta time, and updates UV coordinates
* to display the correct frame from the sprite sheet.
*/
class AnimationSystem {
public:
    /**
    * @brief Updates all sprite animations in the ECS.
    * @param dt Delta time in seconds since the last update.
    * @param ecs Reference to the ECS instance managing entities and components.
    * @param renderer Reference to the Renderer used for drawing (not directly modified here but may be used for GPU sync).
    * @param tex Reference to TextureStore for retrieving textures if needed.
    * @note Only entities with both SpriteAnimationComponent and ImageRenderer are updated.
    */
    void Update(float dt, ECS& ecs, Renderer& renderer, TextureStore& tex);
};
