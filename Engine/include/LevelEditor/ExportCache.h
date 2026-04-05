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
	
	/*!*************************************************************************
	Class for ExportCache
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
