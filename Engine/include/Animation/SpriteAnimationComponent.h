/*!------------------------------------------------------------------------------------------------
	File: SpriteAnimationComponent.cpp
	Project: NocturnalStudio

	This file contains the SpriteAnimationComponent struct with animation data for sprite entities.

	Primary Author:
		 Chang Qi Xuan Celest
		- Created SpriteAnimationComponent struct.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/
#pragma once
#include <unordered_map>
#include <vector>
#include <string>

/**
* @struct SpriteAnimationComponent
* @brief Stores all animation data for a sprite entity.
* Each entity with a sprite animation has a SpriteAnimationComponent that
* tracks the current animation, current frame, timer, and sheet layout.
*/
struct SpriteAnimationComponent {
    /**
    * @struct Animation
    * @brief Defines a single animation sequence.
    */
    struct Animation {
		int frames; //number of frames in the animation
		float frameTime; //time per frame in seconds
		bool loop; //whether the animation loops

        // NEW: per-animation spritesheet + layout
        std::string spritePath;   // e.g. "assets/textures/idle.png" or "textures/idle.png"
        int sheetColumns = 1;
        int sheetRows = 1;
        unsigned int cachedTexture = 0; // lazy-filled by AnimationSystem
    };

	std::unordered_map<std::string, Animation> animations; //map of animation name to Animation data
	std::string currentAnim; //name of the current animation
	int currentFrame = 0; //current frame index
	float timer = 0.0f; //timer to track frame time
	bool playing = true; //is the animation playing
    // Keep for backward compatibility (used as fallback if per-animation cols/rows unset)
    int sheetColumns = 1;
    int sheetRows = 1;
};
