/*!------------------------------------------------------------------------------------------------
	File: ExportCache.h
	Project: NocturnalStudio

	Brief:
		This file contains the class for ExportCache

	Primary Author:
		 LAYCOCK ENHUI SARAH (100%)
		- Created the ExportCache.h file and its function specification.

	All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Serialization/JSONserialization.h"


namespace NocturnalEngine
{
	//When Buiilding a New Component
	//Ensure that the SceneManager Init has the component registered
	//Ensure that the Serialize and De-Serialize functions in the SceneManager has those components checked
	/*!*************************************************************************
	Interface Class for IComponent
	****************************************************************************/
	class ExportCache : public JSONSerializer
	{
	public:
		virtual bool Deserialize(const rapidjson::Value& obj);
		virtual bool Serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>* writer) const ;

		std::map<std::filesystem::path, bool>& GetCache() { return mbuildCache; }

	protected:
		std::map<std::filesystem::path, bool> mbuildCache{};
	};
}
