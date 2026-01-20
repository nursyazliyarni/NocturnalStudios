/*!------------------------------------------------------------------------------------------------
    File: IComponent.h
    Project: NocturnalStudio

	Brief:
		This file contains the abstract class for IComponent
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the IComponent.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Serialization/JSONserialization.h"
#include "pch.h"
#include "Math/Vmath.h"
#include "ECS/Types.h"

namespace NocturnalEngine
{
	//When Buiilding a New Component
	//Ensure that the SceneManager Init has the component registered
	//Ensure that the Serialize and De-Serialize functions in the SceneManager has those components checked
	/*!*************************************************************************
	Interface Class for IComponent
	****************************************************************************/
	class IComponent : public JSONSerializer
	{
	public:
		virtual ~IComponent() = default;
		virtual void ToEditor() {}
		virtual bool Deserialize(const rapidjson::Value& obj) = 0;
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const = 0;

		/*!*************************************************************************
		Retrieves Component Entity ID
		****************************************************************************/
		virtual Entity& GetComponentEntityID() = 0;
		/*!*************************************************************************
		Sets Components Entity ID
		****************************************************************************/
		virtual void SetComponentEntityID(Entity& entity) = 0;

	protected:
		Entity entityID{};
	};
}