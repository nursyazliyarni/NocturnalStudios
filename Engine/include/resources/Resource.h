/*!------------------------------------------------------------------------------------------------
    File: Resource.h
    Project: NocturnalStudio

    This file contains Resource class declarations.

    Primary Author:
		Lau Xiang Yang (100%): 
		- Resource class definition.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once

#include <string>
#include <numeric>

using ResourceID = unsigned;
constexpr ResourceID NEW_RESOURCE = std::numeric_limits<ResourceID>::max();
constexpr ResourceID INVALID_RESOURCE_ID = std::numeric_limits<ResourceID>::max();
constexpr ResourceID NO_RESOURCE_ID = std::numeric_limits<ResourceID>::max();
constexpr ResourceID NO_RESOURCE_SELECTED_ID = std::numeric_limits<ResourceID>::max();

/**
 * @brief Represents a generic resource managed by the engine.
 *
 * A Resource holds an identifier, a key (logical name) and a file path to the
 * underlying asset. Copying is disabled to avoid accidental duplication of
 * resource identity; move semantics are permitted.
 */
class Resource {
public:
    /**
     * @brief Construct a new Resource.
     * @param id Unique identifier for the resource (use NEW_RESOURCE for new/unsaved).
     * @param key Logical name or key for the resource.
     * @param filePath Filesystem path to the resource asset.
     */
    Resource(ResourceID id, const std::string& key, const std::string& filePath);

    /**
     * @brief Get the file path for this resource.
     * @return const std::string& Reference to the file path string.
     */
    std::string const& getFilePath() const;

    /**
     * @brief Set the file path for this resource.
     * @param filePath New filesystem path to store.
     */
    void setFilePath(const std::string& filePath);

    /**
     * @brief Get the resource key (logical name).
     * @return const std::string& Reference to the key string.
     */
    std::string const& getKey() const;

    /**
     * @brief Set the resource key (logical name).
     * @param key New key to assign.
     */
    void setKey(const std::string& key);

    /**
     * @brief Get the resource identifier.
     * @return ResourceID const& Reference to the identifier value.
     */
    ResourceID const& getID() const;

    // Disable copying: resources should not be duplicated accidentally.
    Resource(const Resource& other) = delete;
    Resource& operator=(const Resource& other) = delete;

    // Allow moves for transferring ownership/identity.
    Resource(Resource&& other) = default;
    Resource& operator=(Resource&& other) = default;

protected:
    ResourceID id;         /**< Numeric identifier for the resource. */
    std::string key;       /**< Logical key/name for the resource. */
    std::string filePath;  /**< Filesystem path to the asset. */
};