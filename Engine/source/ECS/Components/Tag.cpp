/*!------------------------------------------------------------------------------------------------
    File: Tag.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize tag components for ECS.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the tag class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/Tag.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Ctor for Tag Component
	****************************************************************************/
	Tag::Tag() : mTag{"Untagged"}, mTargetTag{""} {}

	/*!*************************************************************************
	Deserialize for Tag Component
	****************************************************************************/
	bool Tag::Deserialize(const rapidjson::Value& obj)
	{
		mTag = obj["Tag"].GetString();
		mTargetTag = obj["TargetTag"].GetString();
		return true;
	}

	/*!*************************************************************************
	Serialize for Tag Component
	****************************************************************************/
	bool Tag::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
	{
		writer->Key("Tag");
		writer->String(mTag.c_str());
		writer->Key("TargetTag");
		writer->String(mTargetTag.c_str());
		return true;
	}
}