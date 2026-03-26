/*!------------------------------------------------------------------------------------------------
    File: Transform.h
    Project: NocturnalStudio

	Brief:
		Transform component used to set the pos, scale and rot of the
		entity and used for various systems such as graphics to render images
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the Transform.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Math/Vmath.h"
#include "IComponent.h"

namespace NocturnalEngine
{
	class Transform : public IComponent
	{
	public:
		Transform();
		~Transform() = default;
		virtual bool Deserialize(const rapidjson::Value & obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>*writer) const;
		bool ToEditor()override;

		vec2D& GetPos() { return mPosition; }
		vec2D& GetScale() { return mScale; }
		float& GetRot() { return mRot; }

		void SetPos(vec2D Pos) { mPosition = Pos; }
		void SetPos(float posX, float posY) { mPosition = vec2D(posX, posY); }
		void SetPosFromCenterLeft(vec2D Pos) { mPosition = vec2D(Pos.x + mScale.x / 2.f, Pos.y); }
		void SetPosFromCenterLeft(float posX, float posY) { mPosition = vec2D(posX + mScale.x / 2.f, posY); }
		void SetScale(vec2D Scale) { mScale = Scale; }
		void SetScale(float X, float Y) { mScale = vec2D(X, Y); }
		void SetRot(float value) { mRot = value; }
		void MoveY(float y) { mPosition.y += y; }
		void MoveX(float x) { mPosition.x += x; }
		void ScaleBy(float x) { mScale.x *=x; mScale.y *= x; }


		vec2D GetMinPos() { return {mPosition.x-mScale.x/2.f,mPosition.y - mScale.y / 2.f}; }
		vec2D GetMaxPos() { return { mPosition.x + mScale.x / 2.f,mPosition.y + mScale.y / 2.f }; }

		void UnparentPos(bool precheck=false);
		void ParentPos(bool precheck = false);

		vec2D GetWorldPos();
		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		Entity& GetComponentEntityID() { return entityID; }
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		void SetComponentEntityID(Entity& entity) { entityID = entity; }

	private:
		vec2D mPosition;
		vec2D mScale;
		float mRot;

		bool isLocalSpace{ false };

		vec2D GetVec2d(const rapidjson::Value&);
	};
}
