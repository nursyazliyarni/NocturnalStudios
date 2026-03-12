/*!------------------------------------------------------------------------------------------------
    File: Animation.cpp
    Project: NocturnalStudio

	Brief:
		This file sets the frame data for each sprite based on delta time to create animation.
	
    Primary Author:
         Chang Qi Xuan Celest (100%)
		- Created the animation class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Animation/Animation.h"
#include "ResourceManager/ResourceManager.h"


namespace NocturnalEngine {
	std::unordered_map<std::string, std::vector<float>> Animation::spriteContainer;
	/*!*************************************************************************
	Constructor for animation
	****************************************************************************/
	Animation::Animation()
	{

	}

	/*!*************************************************************************
	Update animations based on delta time
	****************************************************************************/
	void Animation::UpdateAnimation(float deltatime, Sprite& sprite)
	{
		sprite.internaltimer += deltatime;

		const std::string textureKey = sprite.GetTexture();
		// Skip animation work when no valid texture is assigned.
		if (textureKey.empty() || !ResourceManager::GetTexture(textureKey))
			return;

		// Frame count is inferred from spritesheet width (512px per frame).
		int maxFrames = static_cast<int>(GETTEXTURE(textureKey)->GetWidth() / 512.f);
		if (maxFrames <= 0)
			maxFrames = 1;
		sprite.GetMaxIndex() = maxFrames;

		// Ensure timing data exists and matches the detected frame count.
		auto& times = spriteContainer[textureKey];
		if (times.empty())
		{
			times = sprite.GetDisplayTime();
		}
		if (times.empty())
		{
			times.resize(maxFrames, 0.1f);
		}
		else if (static_cast<int>(times.size()) != maxFrames)
		{
			times.resize(maxFrames, 0.1f);
		}

		if (sprite.GetIndex().x < 0 || sprite.GetIndex().x >= maxFrames)
			sprite.GetIndex().x = 0;

		// Guard against invalid/zero frame durations.
		float frameDuration = times[sprite.GetIndex().x];
		if (frameDuration <= 0.0f)
			frameDuration = 0.1f;

		if (sprite.internaltimer >= frameDuration)
		{
			sprite.GetIndex().x++;
			sprite.internaltimer = 0.0f;
			if (sprite.GetIndex().x >= maxFrames)
			{
				sprite.GetIndex().x = 0;
			}
		}
	}

	std::string Animation::ResolveStateTexture(const Sprite& sprite,
		double hungerRatio, double healthRatio, double sanityRatio, bool usePreviewMode)
	{
		if (!sprite.stateManagerEnabled || sprite.stateRules.empty())
			return "";

		int selectedRule = -1;

		// Editor preview bypasses rule evaluation and picks the selected rule directly.
		if (usePreviewMode &&
			sprite.statePreviewMode > 0 &&
			sprite.statePreviewMode <= static_cast<int>(sprite.stateRules.size()))
		{
			selectedRule = sprite.statePreviewMode - 1;
		}
		else
		{
			// Auto mode: pick the highest-priority rule that matches.
			int bestPriority = -2147483647;
			for (int i = 0; i < static_cast<int>(sprite.stateRules.size()); ++i)
			{
				const auto& rule = sprite.stateRules[i];
				if (!rule.enabled)
					continue;

				double sourceValue = 0.0;
				bool matched = false;
				switch (rule.source)
				{
				case 0: // Hunger
					sourceValue = hungerRatio;
					matched = (rule.compare == 0) ? (sourceValue <= rule.threshold) : (sourceValue >= rule.threshold);
					break;
				case 1: // Health
					sourceValue = healthRatio;
					matched = (rule.compare == 0) ? (sourceValue <= rule.threshold) : (sourceValue >= rule.threshold);
					break;
				case 2: // Sanity
					sourceValue = sanityRatio;
					matched = (rule.compare == 0) ? (sourceValue <= rule.threshold) : (sourceValue >= rule.threshold);
					break;
				case 3: // Always
				default:
					matched = true;
					break;
				}

				if (!matched)
					continue;

				if (selectedRule < 0 || rule.priority >= bestPriority)
				{
					bestPriority = rule.priority;
					selectedRule = i;
				}
			}
		}

		if (selectedRule < 0 || selectedRule >= static_cast<int>(sprite.stateRules.size()))
			return "";

		const auto& chosen = sprite.stateRules[selectedRule];
		if (chosen.textureKey.empty())
			return "";
		if (!ResourceManager::GetTexture(chosen.textureKey))
			return "";

		return chosen.textureKey;
	}

}
