#pragma once
#include "ECS/ECS.h"
#include "Animation/SpriteAnimationComponent.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"

class TextureStore;
class AnimationSystem {
public:
    void Update(float dt, ECS& ecs, Renderer& renderer, TextureStore& tex);
};
