/*!------------------------------------------------------------------------------------------------
	File: ExportCache.cpp
	Project: NocturnalStudio

	Brief:
		This file contains the serialization and deserialization for ExportCache

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the ExportCache.cpp file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "LevelEditor/ExportCache.h"


namespace NocturnalEngine
{

	/*!*************************************************************************
	Deserializes export cache
	****************************************************************************/
	bool ExportCache::Deserialize(const rapidjson::Value& obj) {
		if (!obj.IsArray())return false;

		for (const rapidjson::Value& o : obj.GetArray()) {
			if (!o.HasMember("path") || !o["path"].IsString())continue;
			if (!o.HasMember("selected") || !o["selected"].IsBool())continue;
			mbuildCache.emplace(std::filesystem::path{ o["path"].GetString() }, o["selected"].GetBool());

		}
		return true;
	}
	/*!*************************************************************************
	Serializes export cache
	****************************************************************************/
	bool ExportCache::Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const {
		writer->StartArray();
		for (const std::pair< std::filesystem::path, bool>& pair : mbuildCache) {
			writer->StartObject();
			writer->Key("path");
			writer->String(pair.first.string().c_str());
			writer->Key("selected");
			writer->Bool(pair.second);
			writer->EndObject();
		}
		writer->EndArray();
		return true;
	}



}
