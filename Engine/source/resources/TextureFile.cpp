/*!------------------------------------------------------------------------------------------------
    File: TextureFile.cpp
    Project: NocturnalStudio

    This file contains TextureFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- TextureFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "resources/TextureFile.h"
#include "core/Logger.h"

extern Logger logger;

/**
 * @brief Construct a new TextureFile resource.
 * 
 * @param id Unique identifier for the texture resource.
 * @param key Logical name or key for the texture resource.
 * @param filePath Filesystem path to the texture asset.
 */
TextureFile::TextureFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("TextureFile resource created: " + key + " from " + filePath);
}