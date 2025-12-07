/*!------------------------------------------------------------------------------------------------
    File: ShaderFile.h
    Project: NocturnalStudio

    This file contains ShaderFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- ShaderFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "resources/ShaderFile.h"
#include "core/Logger.h"

extern Logger logger;

/**
 * @brief Construct a new Shader File:: Shader File object
 * 
 * @param id  ResourceID assigned to this resource.
 * @param key   readable name or key for the resource.
 * @param filePath  Filesystem path to the resource asset.
 */
ShaderFile::ShaderFile(ResourceID id, const std::string& key, const std::string& filePath)
	: Resource(id, key, filePath) {
	logger.log("ShaderFile resource created: " + key + " from " + filePath);
}