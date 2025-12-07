/*!------------------------------------------------------------------------------------------------
    File: ShaderFile.h
    Project: NocturnalStudio

    This file contains ShaderFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- ShaderFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Resource.h"

/**
 * @brief Resource wrapper for a shader file.
 * 
 * ShaderFile represents a shader resource that is tracked by the resource management system.
 * It stores the ResourceID, key, and the original file path used to create the shader.
 * 
 */
class ShaderFile : public Resource {
public:
	/**
	 * @brief Construct a new Shader File object
	 * 
	 * @param id  ResourceID assigned to this resource.
	 * @param key  readable name or key for the resource.
	 * @param filePath  Filesystem path to the resource asset.
	 */
	ShaderFile(ResourceID id, const std::string& key, const std::string& filePath);

	ShaderFile(ShaderFile const& other) = delete;
	ShaderFile& operator=(ShaderFile const& other) = delete;
	ShaderFile(ShaderFile&& other) = default;
	ShaderFile& operator=(ShaderFile&& other) = default;
};
