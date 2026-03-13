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
#include <string>
#include <vector>

namespace NocturnalEngine {

	class Sprite : public IComponent
	{
	public:
		struct AnimationStateRule
		{
			// User-facing state label shown in editor preview dropdown.
			std::string name{ "State" };
			// Texture key from ResourceManager::textures.
			std::string textureKey{};
			// Per-state spritesheet frame size in pixels.
			float uSize{ 512.0f };
			float vSize{ 512.0f };
			// 0=Hunger, 1=Health, 2=Sanity, 3=Always.
			int source{ 3 };
			// 0=<= threshold, 1=>= threshold.
			int compare{ 0 };
			// Normalized threshold [0..1].
			float threshold{ 0.5f };
			// Higher number wins when multiple states match.
			int priority{ 0 };
			bool enabled{ true };
		};

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

		int& Get_SPRITE_type() { return SPRITE_type; }
		void Set_SPRITE_type(int enum_sprite_type) { SPRITE_type = enum_sprite_type; }

		int& Get_SHAPE_type() { return SHAPE_type; }
		void Set_SHAPE_type(int enum_shape_type) { SHAPE_type = enum_shape_type; }

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
		// General animation state manager settings shown in Sprite component editor.
		bool stateManagerEnabled{ false };
		bool statePreviewInEditor{ false };
		// 0=Auto, n>0 maps to stateRules[n-1].
		int statePreviewMode{ 0 };
		std::vector<AnimationStateRule> stateRules{};
		
		//altas 
	private:

		int SPRITE_type;
		int SHAPE_type;
		float circle_radius;
		std::array<float, 4> rgba_color;
		std::string mTextureName;
		vec2D mUVcooridnates;
		float mAlpha;
		int MaxIndex_X;
		std::vector<float>displayTime;

		enum sprite_type {
			texture, shape, sprite_type_count
		};
		enum shape_type {
			rectangle, circle, shape_type_count
		};
		const char* sprite_type_ToString(int enum_sprite_type);
		const char* shape_type_ToString(int enum_shape_type);
		friend class Graphic;
	};
}
