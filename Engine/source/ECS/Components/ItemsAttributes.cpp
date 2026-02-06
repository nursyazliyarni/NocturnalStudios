/*!------------------------------------------------------------------------------------------------
    File: ItemsAttributes.cpp
    Project: NocturnalStudio

	Brief:
		This file is used to serialize and deserialize items attributes components for ECS.
	
    Primary Author:
         Ang Yu Xuan (100%)
		- Created the items attributes class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "ECS/Components/ItemsAttributes.h"

NocturnalEngine::ItemsAttributes::ItemsAttributes() :type{ ItemType::FOOD}, gain_amount { 0 }, is_dragging{ false } {}

bool NocturnalEngine::ItemsAttributes::Deserialize(const rapidjson::Value& obj)
{
	if(obj.HasMember("type"))type = static_cast<ItemType>(obj["type"].GetInt());
	gain_amount = obj["gain amount"].GetDouble();
	is_dragging = obj["is_dragging"].GetBool();

	return true;
}

bool NocturnalEngine::ItemsAttributes::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const
{
	writer->Key("type");
	writer->Int(static_cast<int>(type));
	writer->Key("gain amount");
	writer->Double(gain_amount);
	writer->Key("is_dragging");
	writer->Bool(is_dragging);
	return true;
}
