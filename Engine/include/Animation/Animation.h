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
		void UpdateShapeAnimation(float deltatime, Sprite& sprite);
		// Resolve the texture key from sprite state rules using normalized status ratios.
		static int ResolveStateRuleIndex(const Sprite& sprite,
			double hungerRatio, double healthRatio, double sanityRatio, bool usePreviewMode);
		static std::string ResolveStateTexture(const Sprite& sprite,
			double hungerRatio, double healthRatio, double sanityRatio, bool usePreviewMode);
		static bool ApplyStateRule(Sprite& sprite, int ruleIndex);
	private:
	};
}
