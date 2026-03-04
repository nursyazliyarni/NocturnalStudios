/*!------------------------------------------------------------------------------------------------
    File: Sprite.h
    Project: NocturnalStudio

	Brief:
 		This file contain the declaration function for Sprite class
		which is the component for sprite rendering
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Sprite.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "glm/glm.hpp"

namespace NocturnalEngine {

	class Sprite : public IComponent
	{
	public:
		struct Coordinates
		{
			int x; int y;
			
		}Index;
		
		Sprite();
		~Sprite() = default;
		virtual bool Deserialize(const rapidjson::Value & obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>*writer) const;
		bool ToEditor()override;

		void SetTexture(const std::string& name) { mTextureName = name; }
		std::string &GetTexture() { return mTextureName; }

		void SetIndex(int& x, int& y) { Index.x = x, Index.y = y; } // y will always remind as zero
		Coordinates& GetIndex() { return Index; }

		std::vector<float>&GetDisplayTime() { return displayTime; }
		int& GetMaxIndex() { return MaxIndex_X; }
		void SetUVCoor(float& u, float& v) { mUVcooridnates.x = u; mUVcooridnates.y = v; } 
		vec2D& GetUVCoor() { return mUVcooridnates; }

		void SetAlpha(float in) { mAlpha = in; }
		float GetAlpha() { return mAlpha; }

		void SetLayerOrder(int layer) { LayerOrder = layer; }
		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	public:
	
		bool is_SpriteSheet; // to tell whether we are using a spritesheet
		bool is_Animated;	// if is a spritesheet do we want it to be animated 
		float internaltimer; // internal timer for animation
		int LayerOrder; // layering order for rendering
		//altas 
	private:
		std::string mTextureName;
		vec2D mUVcooridnates;
		float mAlpha;
		int MaxIndex_X;
		std::vector<float>displayTime;
	};
}
