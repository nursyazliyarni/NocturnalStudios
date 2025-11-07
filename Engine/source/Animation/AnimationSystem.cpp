/*!------------------------------------------------------------------------------------------------
    File: AnimationSystem.cpp
    Project: NocturnalStudio

	This file contains Font functions definitions for animation system updating sprite animations.

    Primary Author:
         Chang Qi Xuan Celest
		- Created AnimationSystem function definitions.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#include "Animation/AnimationSystem.h"
#include "Animation/SpriteAnimationComponent.h"
#include "ECS/ECS.h"
#include "graphics/Renderer.h"
#include <iostream>

/**
* @brief Updates all sprite animations and applies current frame to ImageRenderer UVs.
* This function iterates over all entities in the ECS. For each entity:
* - Checks for a SpriteAnimationComponent and verifies it is playing.
* - Advances the animation timer based on `dt`.
* - Advances the current frame if the timer exceeds the frame time.
* - Handles looping or stopping the animation.
* - Updates the `uvMin` and `uvMax` in the associated ImageRenderer to display
*   the correct frame from the sprite sheet.
* @param dt Delta time in seconds since the last update.
* @param ecs Reference to the ECS managing entities and components.
* @param renderer Reference to the Renderer.
* @param tex Reference to TextureStore (currently unused in this function but may be used for texture lookups).
*/
void AnimationSystem::Update(float dt, ECS& ecs, Renderer& renderer, TextureStore& texStore)
{
    for (EntityID id : ecs.getAllEntities()) {
        auto* anim = ecs.getComponent<SpriteAnimationComponent>(id);
        if (!anim || !anim->playing)
            continue;

        auto it = anim->animations.find(anim->currentAnim);
        if (it == anim->animations.end())
            continue;

        auto& current = it->second;
        anim->timer += dt;

        anim->timer += dt;
        if (anim->timer >= current.frameTime) {
            anim->timer -= current.frameTime;
            anim->currentFrame++;

            if (anim->currentFrame >= current.frames) {
                if (current.loop)
                    anim->currentFrame = 0;
                else {
                    anim->currentFrame = current.frames - 1;
                    anim->playing = false;
                }
            }
        }

        auto* img = ecs.getComponent<ImageRenderer>(id);
        if (!img)
            continue;

		//calculate UV coordinates for the current frame
        int frameIndex = anim->currentFrame;
        float uPerFrame = 1.0f / anim->sheetColumns;
        float vPerFrame = 1.0f / anim->sheetRows;

        int col = frameIndex % anim->sheetColumns;
        int row = frameIndex / anim->sheetColumns;

        img->uvMin = glm::vec2(col * uPerFrame, row * vPerFrame);
        img->uvMax = glm::vec2(img->uvMin.x + uPerFrame, img->uvMin.y + vPerFrame);
    }
}
