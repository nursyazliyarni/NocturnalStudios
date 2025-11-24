/*!------------------------------------------------------------------------------------------------
    File: Resource.cpp
    Project: NocturnalStudio

    This file contains Resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "../include/resources/Resource.h"

/**
 * @brief Construct a new Resource:: Resource object
 * 
 * This constructor initializes a Resource object with the provided
 * identifier, key, and file path.
 * 
 * @param id  Unique identifier for the resource (use NEW_RESOURCE for new/unsaved).
 * @param key  Logical name or key for the resource.
 * @param path Filesystem path to the resource asset.
 */
Resource::Resource(ResourceID id, const std::string& key, const std::string& path) :
	id{ id }, key{ key }, filePath{ path } {}

/**
 * @brief Get the resource key (logical name).
 * @return const std::string& Reference to the key string.
 */
const std::string& Resource::getKey() const {
	return key;
}

/**
 * @brief Set the resource key (logical name).
 * @param key New key to assign.
 */
void Resource::setKey(const std::string& val) {
	key = val;
}

/**
 * @brief Get the resource identifier.
 * @return ResourceID const& Reference to the identifier value.
 */
ResourceID const& Resource::getID() const {
	return id;
}

/**
 * @brief Get the file path for this resource.
 * @return const std::string& Reference to the file path string.
 */
const std::string& Resource::getFilePath() const {
	return filePath;
}

/**
 * @brief Set the file path for this resource.
 * @param filePath New filesystem path to store.
 */
void Resource::setFilePath(const std::string& val) {
	filePath = val;
}
