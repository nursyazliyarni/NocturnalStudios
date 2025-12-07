/*!------------------------------------------------------------------------------------------------
    File: PrefabFile.h
    Project: NocturnalStudio

    This file contains PrefabFile resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Prefab resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Resource.h"

/**
 * @brief Represents a Prefab resource in the engine.
 * 
 * The PrefabFile class inherits from the Resource base class and encapsulates
 * the properties and behaviors specific to prefab resources. It utilizes the
 * ResourceID, key, and filePath to uniquely identify and manage prefab assets
 * within the engine.
 */
class PrefabFile : public Resource {
public:
	/**
	 * @brief Construct a new Prefab File object
	 * 
	 * @param id Resource ID
	 * @param key Lookup key for the resource
	 * @param filePath Filesystem path to the prefab file
	 */
	PrefabFile(ResourceID id, const std::string& key, const std::string& filePath);
};
