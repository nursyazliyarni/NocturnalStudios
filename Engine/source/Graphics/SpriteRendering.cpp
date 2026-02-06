/*!------------------------------------------------------------------------------------------------
    File: SpriteRendering.cpp
    Project: NocturnalStudio

	Brief:
		This file render sprite using texture atlas and glm
	
    Primary Author:
         Rishika Bimal Attodi (100%)
		- Created the sprite rendering class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/SpriteRendering.h"

namespace NocturnalEngine {

	/*!*************************************************************************
	Render sprite using glm
	****************************************************************************/
	SpriteRender::SpriteRender(const MultiRefs<Texture>& texture, const glm::vec2& min, const glm::vec2& max)
		:mTexture(texture)
	{
		mTexCoords[0] = { min.x, min.y }; //bottom left
		mTexCoords[1] = { max.x, min.y }; //bottom right
		mTexCoords[2] = { max.x, max.y }; //top right
		mTexCoords[3] = { min.x, max.y }; //top left
	}

	/*!*************************************************************************
	Create sprite using glm
	****************************************************************************/
	MultiRefs<SpriteRender> SpriteRender::CreateSprite(const MultiRefs<Texture>& texture, const glm::vec2& Coordinates, const glm::vec2& SpriteSize)
	{
		glm::vec2 min = { (Coordinates.x * SpriteSize.x) / texture->GetWidth(), (Coordinates.y * SpriteSize.y) / texture->GetHeight() };
		glm::vec2 max = { ((Coordinates.x + 1) * SpriteSize.x) / texture->GetWidth(), ((Coordinates.y + 1) * SpriteSize.y) / texture->GetHeight() };
		
		return CreateMultiRefs<SpriteRender>(texture, min, max);
	}
}
