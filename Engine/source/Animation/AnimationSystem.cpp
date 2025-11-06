#include "Animation/AnimationSystem.h"
#include "Animation/SpriteAnimationComponent.h"
#include "ECS/ECS.h"
#include "graphics/Renderer.h"
#include <iostream>

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
        if (current.frames.empty())
            continue;

        anim->timer += dt;
        if (anim->timer >= current.frameTime) {
            anim->timer -= current.frameTime;
            anim->currentFrame++;

            if (anim->currentFrame >= static_cast<int>(current.frames.size())) {
                if (current.loop)
                    anim->currentFrame = 0;
                else {
                    anim->currentFrame = static_cast<int>(current.frames.size()) - 1;
                    anim->playing = false;
                }
            }
        }

        auto* img = ecs.getComponent<ImageRenderer>(id);
        if (!img)
            continue;

        int frameIndex = current.frames[anim->currentFrame];
        float uPerFrame = 1.0f / anim->sheetColumns;
        float vPerFrame = 1.0f / anim->sheetRows;

        int col = frameIndex % anim->sheetColumns;
        int row = frameIndex / anim->sheetColumns;

        img->uvMin = glm::vec2(col * uPerFrame, row * vPerFrame);
        img->uvMax = glm::vec2(img->uvMin.x + uPerFrame, img->uvMin.y + vPerFrame);
    }
}
