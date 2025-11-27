/*!------------------------------------------------------------------------------------------------
    File: TextureFile.h
    Project: NocturnalStudio

    This file contains TextureFile class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- TextureFile class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include "Resource.h"

/**
 * @brief Resource wrapper for a texture file.
 * 
 * This class represents a texture file resource in the engine, inheriting from the base Resource class.
 * It encapsulates properties and behaviors specific to texture files.
 */
class TextureFile : public Resource {
	public:
	/**
	 * @brief Construct a new TextureFile resource.
	 * 
	 * @param id Unique identifier for the texture resource.
	 * @param key Logical name or key for the texture resource.
	 * @param filePath Filesystem path to the texture asset.
	 */
	TextureFile(ResourceID id, const std::string& key, const std::string& filePath);

	TextureFile(TextureFile const& other) = delete;
	TextureFile& operator=(TextureFile const& other) = delete;
	TextureFile(TextureFile&& other) = default;
	TextureFile& operator=(TextureFile&& other) = default;
};
