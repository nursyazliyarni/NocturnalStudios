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
	
		std::unordered_map <std::string, std::vector <float>> ::const_iterator got = spriteContainer.find(sprite.GetTexture());
		if (got == spriteContainer.end())// always check the sprite container whether there is any data for the current texture. if no, prompt the user
		{
			spriteContainer.emplace(sprite.GetTexture(), sprite.GetDisplayTime());
		}
		else
		{
			sprite.GetMaxIndex() = static_cast<int>(GETTEXTURE(sprite.GetTexture())->GetWidth() / 512.f);
			if (sprite.internaltimer >= spriteContainer[sprite.GetTexture()][sprite.GetIndex().x])
			{
				sprite.GetIndex().x++;
				sprite.internaltimer = 0.0f;
				if (sprite.GetIndex().x >= sprite.GetMaxIndex())
				{
					sprite.GetIndex().x = 0;
				}
			}
		}
	}

}