/*!------------------------------------------------------------------------------------------------
    File: CameraComponent.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize camera components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the camera component class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/CameraComponent.h"

namespace NocturnalEngine {

	/*!*************************************************************************
	Constructor for camera component
	****************************************************************************/
	CameraComponent::CameraComponent()
	{
	}

	/*!*************************************************************************
	Derserialize camera component
	****************************************************************************/
	bool CameraComponent::Deserialize(const rapidjson::Value& obj)
	{
		UNREFERENCED_PARAMETER(obj);
		return false;
	}

	/*!*************************************************************************
	Serialize camera component
	****************************************************************************/
	bool CameraComponent::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		UNREFERENCED_PARAMETER(writer);
		return false;
	}
}