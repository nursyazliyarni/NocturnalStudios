/*!------------------------------------------------------------------------------------------------
    File: Collider.h
    Project: NocturnalStudio

	Brief:
		This file contains the component declaration as well as member function
		definitions for the collider component of the game engine.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Collider.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "IComponent.h"
#include "Math/Vmath.h"

namespace NocturnalEngine
{
	class Collider : public IComponent
	{
	public:
		/*!*************************************************************************
		This is the list of collider types. Collider type also informs response behavior
		****************************************************************************/
		enum class ColliderType
		{
			none,
			circle, //-------Circle
			bubble, //-------Non-physics Circle
			line, //---------LineSegment
			rect, //---------AABB
			box, //----------Playable area box
			bossball, //-----Just something for fun
			button,
			dashCirc
		};

		struct Colpiece {
			ColliderType mCol{};
			vec2D mCollisionNormal{};
			vec2D mMin{};
			vec2D mMax{};
			vec2D mOffset{};
			int mHit{};
			float mRadius{};
			bool is_Alive{};
		};

		Collider();
		~Collider() = default;

		bool ToEditor()override;
		/*!*************************************************************************
		This function de-serializes the level colliders from the given level json file
		****************************************************************************/
		virtual bool Deserialize(const rapidjson::Value& obj);
		/*!*************************************************************************
		This function saves the level information by serializing it to the level json 
		file
		****************************************************************************/
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const;
    
		Colpiece& operator[](int i) {
			return mColArr[i];
		}

		Colpiece* GetCollisionArray()
		{
			return mColArr;
		}

		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:
		Colpiece mColArr[2]{};
	};
}
