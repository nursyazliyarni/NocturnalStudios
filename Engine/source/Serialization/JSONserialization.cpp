/*!------------------------------------------------------------------------------------------------
    File: JSONserialization.cpp
    Project: NocturnalStudio

	Brief:
        This file contains the implementation of the JSONserialization script class which is responsible 
        for managing JSON serialization within the engine.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the JSONserialization class and its function specification.
		
    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Serialization/JSONserialization.h"
#include "Audio/AudioEngine.h"

namespace NocturnalEngine
{
	/*!*************************************************************************
	Json Serializer for calling Derived class Serialize function
	****************************************************************************/
	std::string JSONSerializer::Serialize() const
	{
		rapidjson::StringBuffer ss;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(ss);
		if (Serialize(&writer)) 
		{ 
			return ss.GetString();
		}
		return "";
	}

	/*!*************************************************************************
	Json Deserializer for calling Derived class Deserialize function
	****************************************************************************/
	bool JSONSerializer::Deserialize(const std::string& stringbuffer)
	{
		rapidjson::Document doc;
		doc.Parse(stringbuffer.c_str());
		if (!InitDocument(stringbuffer, doc))
		{
			return false;
		}
		else
		{
			p_Audio->StopChannel(CAudioEngine::channel_groups::bgm);
			Deserialize(doc);
		}
		return true;
	}

	/*!*************************************************************************
	Funtion for Deserializing to JSON file
	****************************************************************************/
	bool JSONSerializer::DeserializeFromFile(const std::string& filePath)
	{
		std::ifstream ifs(filePath, std::ios::in);
		std::stringstream buffer;
		buffer << ifs.rdbuf();
		ifs.close();

		return Deserialize(buffer.str());
	}

	/*!*************************************************************************
	Funtion for Serializing to JSON file
	****************************************************************************/
	bool JSONSerializer::SerializeToFile(const std::string& filePath)
	{
		std::ofstream ofs(filePath);
		std::string str = Serialize();
		ofs << str;
		ofs.flush();
		ofs.close();

		return true;
	}

	/*!*************************************************************************
	Checks if Document can be Read
	****************************************************************************/
	bool JSONSerializer::InitDocument(const std::string& stringbuffer, rapidjson::Document& doc)
	{
		if (stringbuffer.empty())
			return false;

		std::string validJson(stringbuffer);

		return !doc.Parse(validJson.c_str()).HasParseError() ? true : false;
	}
}