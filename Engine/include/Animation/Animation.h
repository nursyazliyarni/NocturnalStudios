/*!------------------------------------------------------------------------------------------------
    File: Animation.h
    Project: NocturnalStudio

	Brief:
		This file contains the declaration of Animation class.
	
    Primary Author:
         Chang Qi Xuan Celest (100%)
		- Created the animation class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Graphics/Renderer.h"
#include "Graphics/SpriteRendering.h"
#include "Math/Vmath.h"

namespace NocturnalEngine {

	class Animation
	{
	public:
	    static std::unordered_map<std::string, std::vector<float>> spriteContainer;// to store all animation info
		Animation();
		void UpdateAnimation(float deltatime, Sprite& sprite);
	private:
	};
}