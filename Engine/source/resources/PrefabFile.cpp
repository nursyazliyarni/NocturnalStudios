/*!------------------------------------------------------------------------------------------------
    File: PrefabFile.cpp
    Project: NocturnalStudio

    This file contains PrefabFile resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Prefab resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "resources/PrefabFile.h"
#include "core/Logger.h"

extern Logger logger;
/**
 * @brief Construct a new Prefab File and log its creation.
 * 
 * The constructor initializes the PrefabFile resource by calling the base Resource
 * class constructor with the provided id, key, and filePath. It also logs the creation
 * of the PrefabFile resource using the global logger instance.
 * 
 * @param id Resource ID
 * @param key Lookup key for the resource
 * @param filePath Filesystem path to the prefab file
 */
PrefabFile::PrefabFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("PrefabFile resource created: " + key + " from " + filePath);
}
