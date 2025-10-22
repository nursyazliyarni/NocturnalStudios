/* !------------------------------------------------------------------------------------------------
	File: LevelManager.cpp
	Project : NocturnalStudio

	This file contains ...

	Primary Author :
		Name(%)
		- Brief

	Secondary Author :
		Name(%)
		- Brief

All content(c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
* ------------------------------------------------------------------------------------------------ */


#include "./scene/LevelManager.h"


#include "./scene/Prefab.h"
#include "./graphics/Renderer.h"
#include "./math/Rand.h"



EntityPool::EntityPool(size_t capacity) :pool(capacity) {
    for (size_t i = 0; i < capacity; ++i) {
        freeList.push(i);
    }
}

void  EntityPool::Kill(size_t idx){

}

void EntityPool::Render(){
}

void EntityPool::UpdateAll(float dt) {
    // TEMP demo of prefab spawning once (you’ll later refine this per scene)
    PrefabManager prefabMan("assets/prefabs/GameObj.json");

    // Example mapping (you might already have texTagToIndex ready somewhere else)
    std::unordered_map<std::string, size_t> tagToIndex = {
        {"food", 0}, {"drink", 1}, {"toy", 2},
        {"flamingo", 3}, {"horse", 4}, {"owl", 5}, {"seal", 6}
    };

    prefabMan.loadSpriteArchetypes("assets/prefabs/PrefabSprites.json", tagToIndex);

    for (const auto& kv : tagToIndex) {
        const auto* spriteArch = prefabMan.getSprite(kv.first);
        if (!spriteArch) continue;

        float randX = Random_Range(0.1f, 0.8f);
        float randY = Random_Range(0.1f, 0.6f);

        Renderer dummyRenderer("vertex.glsl", "fragment.glsl");
        dummyRenderer.get_sprite_vector().push_back({
            spriteArch->texId,
            randX, randY,
            spriteArch->w > 0 ? spriteArch->w : 0.15f,
            spriteArch->h > 0 ? spriteArch->h : 0.15f,
            Random_Range(0.f, 360.f)
            });
    }
}



// NOTE: Prefab integration placeholder
// In future milestone, LevelManager will:
// 1. Access PrefabManager["Flamingo"] or other names.
// 2. Clone() that prefab into a new GameObject instance.
// 3. Pass its components to Renderer & EntityPool for actual scene rendering.