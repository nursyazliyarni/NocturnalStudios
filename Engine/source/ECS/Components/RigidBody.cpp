/*!------------------------------------------------------------------------------------------------
    File: RigidBody.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize rigid body components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the rigid body class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/RigidBody.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for RigidBody Component
	****************************************************************************/
	RigidBody::RigidBody() : mAccel{vec2D(1.f,1.f)}, mVelocity{vec2D(1.0f,1.0f)}, mDirection{vec2D(1.0f,0.0f)}, mFriction{0.0f}, mRestitution{0.0f} {}

	/*!*************************************************************************
	Deserialize for RigidBody
	****************************************************************************/
	bool RigidBody::Deserialize(const rapidjson::Value& obj)
	{
		mAccel = vec2D(obj["velX"].GetFloat(), obj["velY"].GetFloat());
		mDirection = vec2D(obj["dirX"].GetFloat(), obj["dirY"].GetFloat());
		mFriction = obj["Friction"].GetFloat();
		mRestitution = obj["Restitution"].GetFloat();
		return true;
	}

	/*!*************************************************************************
	Serialize for RigidBody
	****************************************************************************/
	bool RigidBody::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("velX");
		writer->Double(mAccel.x);
		writer->Key("velY");
		writer->Double(mAccel.y);
		writer->Key("dirX");
		writer->Double(mDirection.x);
		writer->Key("dirY");
		writer->Double(mDirection.y);
		writer->Key("Friction");
		writer->Double(mFriction);
		writer->Key("Restitution");
		writer->Double(mRestitution);
		return true;
	}
	//End RigidBody
}